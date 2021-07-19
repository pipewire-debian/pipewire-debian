file (GLOB simple_server_sources ./simple_server.c)

set(SIMPLE_SERVER_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/simple_server CACHE STRING "simple_server dir")
add_executable(simple_server ${simple_server_sources})

target_link_libraries(simple_server openfec m)


file (GLOB simple_client_sources ./simple_client.c)

set(SIMPLE_SERVER_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/simple_client CACHE STRING "simple_client dir")
add_executable(simple_client ${simple_client_sources})

target_link_libraries(simple_client openfec m)
