#!/bin/bash

version="$1"

#re='^[0-9]+$'

if ! [[ $version =~ ^[0-9]+$ ]] ; then
	   echo "error: Please give a version Number correctly" >&2; exit 1
fi


url="https://gitlab.freedesktop.org/pipewire/pipewire/-/archive/0.3.$version/pipewire-0.3.$version.tar.gz"

echo "downloading Pipewire 0.3.$version in your current directory..."

curl -LJO "$url"
