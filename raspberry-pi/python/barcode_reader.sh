#!/bin/bash

LOG="/home/pi/iHammerGo/python/barcode_reader.log"

echo >>"$LOG" 2>&1
echo "------------------------------------------------------------" \
>>"$LOG" 2>&1

echo "Time: $(date "+%Y/%m/%d %H:%M:%S")" >>"$LOG" 2>&1

source /home/pi/.profile
workon cv

config=$(cat /home/pi/iHammerGo/python/barcode_reader.config)

python3 /home/pi/iHammerGo/python/barcode_reader.py $config & \
#>>"$LOG" 2>&1 &

trap 'kill -s SIGINT $!' SIGINT
trap 'kill -s SIGTERM $!' SIGTERM
trap 'kill $!' SIGKILL
wait $!

exit 0
