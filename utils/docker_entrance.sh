#!/bin/bash

SOURCE=${1}

if [[ -z ${1} ]]; then
    SOURCE=${PWD}
fi

docker run \
	--net=host \
	--rm -it \
	--env DISPLAY=${DISPLAY} \
	--privileged \
	-v /dev:/dev \
	-v ${SOURCE}:/entrance \
	wltjr/docker-efl:latest
