file (GLOB_RECURSE openfec_sources *)

# Use -DBUILD_STATIC_LIBS=ON to build static libraries instead of shared.
option(BUILD_STATIC_LIBS "Build the static library" OFF)

if(BUILD_STATIC_LIBS)
  add_library(openfec STATIC ${openfec_sources})
else(BUILD_STATIC_LIBS)
  add_library(openfec SHARED ${openfec_sources})
endif(BUILD_STATIC_LIBS)

# From: $cmake --help-property SOVERSION
#		For shared libraries VERSION and SOVERSION can be used to specify
#		the build version and api version respectively.
#
# Edit it as appropriate to be in line with the src/lib_common/of_openfec_api.c::more_about() version
#
set_target_properties(openfec PROPERTIES
			VERSION 1.4.2
			SOVERSION 1) 

# Feel free to edit as appropriate the "target_link_libraries".
#
# Add the IL (DevIL) library if you want to have the possibility to
# produce H/G matrix images with LDPC codes.
# More precisely, we are relying on the DevIL library
# (http://openil.sourceforge.net/).
# Please install it on your machine before compiling the OpenFEC
# library if needed.
# Otherwise remove the IL library.

target_link_libraries(openfec m)
#target_link_libraries(openfec m IL)

#target_link_libraries(openfec pthread IL)
#target_link_libraries(openfec pthread)
