/*
OpenIO SDS metautils
Copyright (C) 2014 Worldline, as part of Redcurrant
Copyright (C) 2015-2017 OpenIO SAS, as part of OpenIO SDS

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.
*/

#include <errno.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "metautils.h"
#include "volume_lock.h"

static GError*
_check_lock(const char *vol, const char *n, const char *v)
{
	static gssize max_size = 256;
	gchar *buf;
	gssize bufsize, realsize;
	GError *err = NULL;

	bufsize = max_size;
	buf = g_malloc(bufsize);
retry:
	memset(buf, 0, sizeof(bufsize));
	realsize = getxattr(vol, n, buf, bufsize-1);

	if (realsize < 0) {

		if (errno != ERANGE)
			err = NEWERROR(errno, "XATTR get error: %s", strerror(errno));
		else { /* buffer too small */
			bufsize = realsize + 1;
			max_size = 1 + MAX(max_size,bufsize);
			buf = g_realloc(buf, bufsize);
			goto retry;
		}
	}

	if (!err) {
		if (strlen(v) != (gsize)realsize) {
			err = NEWERROR(CODE_INTERNAL_ERROR,
					"XATTR size differ, expected value for %s is %s, got %*s",
					n, v, (int)realsize, buf);
		} else if (0 != memcmp(v, buf, realsize)) {
			err = NEWERROR(CODE_INTERNAL_ERROR,
					"XATTR differ, expected value for %s is %s, got %*s",
					n, v, (int)realsize, buf);
		}
	}

	g_free(buf);
	return err;
}

static GError*
_set_lock(const char *vol, const char *n, const char *v,
	const gboolean servicing)
{
	if (!servicing) {
		int rc = setxattr(vol, n, v, strlen(v), XATTR_CREATE);
		if (!rc)
			return NULL;
	} else {
		errno = EEXIST;
	}
	return (errno == EEXIST) ? _check_lock(vol, n, v)
		: NEWERROR(errno, "XATTR set error: %s", strerror(errno));
}

GError*
volume_service_lock(const char *vol, const char *type, const char *id,
		const char *ns, const gboolean servicing)
{
	EXTRA_ASSERT (vol != NULL);
	EXTRA_ASSERT (ns != NULL);
	EXTRA_ASSERT (id != NULL);
	EXTRA_ASSERT (type != NULL);

	GError *err;
	if (NULL != (err = _set_lock(vol, "user.server.ns", ns, servicing)))
		return err;
	if (NULL != (err = _set_lock(vol, "user.server.id", id, servicing)))
		return err;
	if (NULL != (err = _set_lock(vol, "user.server.type", type, servicing)))
		return err;
	return NULL;
}

