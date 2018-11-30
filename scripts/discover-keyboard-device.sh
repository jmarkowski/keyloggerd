#!/usr/bin/sh
event=$(cat /proc/bus/input/devices | grep -B0 -A10 -i keyboard | grep -Pho "event\d")

echo /dev/input/$event
