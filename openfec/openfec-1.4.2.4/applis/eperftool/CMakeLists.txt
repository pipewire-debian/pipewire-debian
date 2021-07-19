file (GLOB eperftool_sources ./*)

set(EPERFTOOL_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/eperftool CACHE STRING "eperftool dir")
add_executable( eperftool ${eperftool_sources})


target_link_libraries( eperftool openfec m)


