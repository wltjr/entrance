#!/bin/bash

MY_PWD="$(pwd)"

[[ -d build ]] && rm -fr build

CFLAGS=-g
MY_PWD+="/build"
meson \
	--buildtype=debug \
	--prefix "${MY_PWD}" \
	--libdir "${MY_PWD}/test" \
	--sbindir "${MY_PWD}/test" \
	--datadir "${MY_PWD}/test" \
	--sysconfdir "${MY_PWD}/test" \
	-Dcachedir="${MY_PWD}/test" \
	$@ . build
scan-build ninja -C build

# sed to change
# 1. display for xephyr
# 2. connection number
# 3. auth, pid, and log file locations
sed -i -e "s|:0|:1.0|" \
	-e "s|42|43|" \
	-e "s:/var/\(log\|run\):${PWD}/build:g" \
	build/entrance.conf

cd build

DIRS=( test/entrance/themes )

for d in ${DIRS[@]}; do
	[[ ! -d "${d}" ]] && mkdir -p "${d}"
done

[[ ! -L  test/elementary ]] && ln -s /usr/share/elementary test/elementary

cp entrance.conf \
	../data/Xsession \
	src/bin/entrance_client \
	test/entrance

cp data/themes/default/default.edj test/entrance/themes
chmod 664 test/entrance/themes/default.edj
