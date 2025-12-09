#!/bin/bash

# prepare the env
export LD_LIBRARY_PATH=/usr/lib64/
export PKG_CONFIG_PATH=/usr/lib64/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig

# configure and build
meson setup --prefix /usr --buildtype=debug -Dpam=true . build
ninja -C build install

# prevent local X11 conflicts, vt, display, and socket port
sed -i -e "s|7|6|" -e "s|:0|:1|" -e "s|42|42|" /etc/entrance/entrance.conf
