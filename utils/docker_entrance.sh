#!/bin/bash

docker run \
	--net=host \
	--rm -it \
	--env DISPLAY=${DISPLAY} \
	--privileged \
	-v /dev:/dev \
	-v ${1}:/entrance \
	wltjr/docker-efl:latest
