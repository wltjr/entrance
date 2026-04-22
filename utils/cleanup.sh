#!/bin/bash
#
# Script to cleanup any mess resulting from failures during development

killall Xorg

rm -v /tmp/efreetd_dev_*
rm -v /tmp/.ecore_service\|entrance\|*
rm -v -r /tmp/.X11-unix/
rm -v /var/run/entrance.*
rm -v /var/log/entrance.log
