#!/bin/bash

if [[ -z ${1} ]]; then
    echo "missing /path/to/entrance repo"
    exit 1
fi

docker run \
	--net=host \
	--rm -it \
	--env DISPLAY=${DISPLAY} \
	--privileged \
	-v /dev:/dev \
	-v ${1}:/entrance \
	wltjr/docker-efl:latest
