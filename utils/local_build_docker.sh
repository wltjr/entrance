#!/bin/bash

# prepare the env
export LD_LIBRARY_PATH=/usr/lib64/
export PKG_CONFIG_PATH=/usr/lib64/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig

# configure and build
meson setup --prefix /usr --buildtype=debug -Dpam=true . build
ninja -C build install

# prevent local X11 conflicts, vt, display, and socket port
sed -i -e "s|7|9|" -e "s|:0|:1.0|" -e "s|42|43|" \
	-e "s|Enlightenment|Xsession|" /etc/entrance/entrance.conf

