#!/bin/bash
#
# Script to cleanup any mess resulting from failures during development

killall Xephyr Xorg X

rm -v -r /root/.cache/e*
rm -v -r /root/.run
rm -v -r /run/.ecore*
rm -v /tmp/efreetd_*
rm -v -r /tmp/.X11-unix/
rm -v /var/run/entrance.*
rm -v /var/log/entrance.log
rm -v -r /var/cache/entrance/
