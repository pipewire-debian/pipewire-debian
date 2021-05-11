#!/bin/bash


meson configure build  --buildtype=plain \
	--wrap-mode=nodownload --buildtype=plain \
	--prefix=/usr --sysconfdir=/etc --localstatedir=/var \
	--libexecdir=libexec \
    --libdir=lib/x86_64-linux-gnu \
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
	-D volume=enabled 


