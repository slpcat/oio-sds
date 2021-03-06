add_definitions(-DG_LOG_DOMAIN="oio.m0v2" -DTHREADED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")

include_directories(BEFORE
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}
		${CMAKE_BINARY_DIR}
		${CMAKE_BINARY_DIR}/metautils/lib
		${CMAKE_BINARY_DIR}/metautils/asn1c
		${CMAKE_CURRENT_BINARY_DIR})

include_directories(AFTER
		${ZK_INCLUDE_DIRS})

link_directories(
		${ZK_LIBRARY_DIRS}
		${SQLITE3_LIBRARY_DIRS})

add_library(meta0remote SHARED meta0_remote.c meta0_remote.h)
set_target_properties(meta0remote PROPERTIES SOVERSION ${ABI_VERSION})
target_link_libraries(meta0remote
		metautils
		${GLIB2_LIBRARIES})


add_library(meta0utils SHARED meta0_utils.c meta0_utils.h)
set_target_properties(meta0utils PROPERTIES SOVERSION ${ABI_VERSION})
target_link_libraries(meta0utils
		metautils gridcluster
		${GLIB2_LIBRARIES})


add_library(meta0v2 SHARED meta0_backend.h meta0_backend.c)
set_target_properties(meta0v2 PROPERTIES SOVERSION ${ABI_VERSION})
target_link_libraries(meta0v2
		meta0utils metautils gridcluster sqliterepo
		${GLIB2_LIBRARIES} ${SQLITE3_LIBRARIES})


add_executable(meta0_client meta0_client.c)
bin_prefix(meta0_client -meta0-client)
target_link_libraries(meta0_client gridcluster
		meta0remote meta0utils metautils server)


add_executable(meta0_server
		zk_manager.c
		meta0_gridd_dispatcher.c
		meta0_server.c)
bin_prefix(meta0_server -meta0-server)
target_link_libraries(meta0_server
		meta0v2 meta0utils
        metautils gridcluster server sqliterepo
		sqlxsrv
		${GLIB2_LIBRARIES} ${SQLITE3_LIBRARIES})

install(TARGETS meta0utils meta0remote meta0v2 meta0_server meta0_client
		LIBRARY DESTINATION ${LD_LIBDIR}
		RUNTIME DESTINATION bin)

install(TARGETS meta0remote
		LIBRARY DESTINATION ${LD_LIBDIR}
		RUNTIME DESTINATION bin)

