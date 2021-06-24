#!/bin/bash

comp_options=
enable_compiler_options=true
source_dir="$(pwd)"

compiler_options="-g -O2 -fdebug-prefix-map=$source_dir=. -fstack-protector-strong -Wformat -Werror=format-security"

cargs="$compiler_options -Wdate-time -D_FORTIFY_SOURCE=2"
clink_args="$compiler_options -Wl,-Bsymbolic-functions  -Wl,-z,relro -Wl,-z,now -Wl,-z,defs -Wl,--as-needed"

#echo -e "\n\nsource_dir=$source_dir\n\n compiler_options=$compiler_options\n\n cargs=$cargs\n\n clink_args=$clink_args\n\n"


if [[ "$enable_compiler_options" == "true" ]];
then
	comp_options=(
      -D c_args="$cargs" 
      -D c_link_args="$clink_args" 
      -D cpp_args="$cargs"
      -D cpp_link_args="$clink_args"   
    )
else
	comp_options=(
      -D c_args="" 
      -D c_link_args=""
      -D cpp_args=""
      -D cpp_link_args=""
    )
fi

meson configure build  --buildtype=plain \
	--wrap-mode=nodownload \
	--prefix=/usr --sysconfdir=/etc --localstatedir=/var \
	--libexecdir=libexec \
    --libdir=lib/x86_64-linux-gnu \
	"${comp_options[@]}" \
	-D audiotestsrc=enabled \
	-D auto_features=enabled \
	-D b_lto=true \
	-D b_pie=true \
	-D libcamera=disabled \
	-D docs=enabled \
	-D examples=enabled \
	-D installed_tests=enabled \
	-D man=enabled \
	-D test=enabled \
	-D videotestsrc=enabled \
	-D volume=enabled \
    -D roc=disabled


