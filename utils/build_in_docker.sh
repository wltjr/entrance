#!/bin/bash
#
# Script to build all variations of entrance and install one in docker for use
# in development with or without Xephyr. This script is not used in CI.

if [[ ! -f /.dockerenv ]]; then
 	echo "Only meant to be run inside docker container"
 	exit 1
fi

# exit on any command failure
set -e

# build all options
rm -fr build/
meson setup --prefix /usr --buildtype=debug -Dpam=false -Dlogind=false . build
ninja -C build

rm -r build/
meson setup --prefix /usr --buildtype=debug -Dpam=true -Dlogind=false . build
ninja -C build

rm -r build/
meson setup --prefix /usr --buildtype=debug -Dpam=false -Dlogind=true . build
ninja -C build

# configure and build
rm -r build/
meson setup --prefix /usr --buildtype=debug -Dpam=true -Dlogind=true . build
ninja -C build install

# prevent local X11 conflicts, vt, display, and socket port
if [[ "${DISPLAY}" == \:0* ]]; then
    sed -i -e "s|7|9|" -e "s|:0|:1.0|" -e "s|42|43|" /etc/entrance/entrance.conf
fi

# create xsession, directory and desktop file
"$(dirname $0)/create_xsession.sh"

sed -i -e "s|Enlightenment|Xsession|" /etc/entrance/entrance.conf
