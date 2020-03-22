#!/bin/bash

LOG="/home/pi/iHammerGo/python/server.log"

echo >>"$LOG" 2>&1
echo "------------------------------------------------------------" \
>>"$LOG" 2>&1

echo "Time: $(date "+%Y/%m/%d %H:%M:%S")" >>"$LOG" 2>&1

user="$(whoami)"
[[ "$user" != "root" ]] && prefix="sudo"

$prefix python3 /home/pi/iHammerGo/python/server.py >>"$LOG" 2>&1 &

[[ -z "$prefix" ]] && \
trap_cmd="kill -s SIGTERM $!" \
|| \
trap_cmd="sudo killall python3"

trap "$trap_cmd" SIGINT SIGTERM
wait $!

exit 0
