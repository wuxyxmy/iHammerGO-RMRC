#!/bin/bash
echo "Running processes: -----------------------------------------"
ps -e | grep -e "mjpg\|python"

echo
echo "Devices: ---------------------------------------------------"
ls --color=always /dev/video*
ls --color=always /dev/ttyS* /dev/ttyAMA*
ls --color=always /dev/ttyUSB*

echo
echo "Service status: --------------------------------------------"
systemctl status ihammergo_video.service -l

echo
echo "USB: -------------------------------------------------------"
read -t 5 -p "Press <Enter> to continue..."
echo

lsusb -t

echo
echo "Video stream log: ------------------------------------------"
read -t 5 -p "Press <Enter> to continue..."

tail -20 /var/log/ihammer_go/ihammergo_video.log

echo
echo "Server log: ------------------------------------------------"
read -t 5 -p "Press <Enter> to continue..."

tail -20 /home/pi/iHammerGo/python/server.log

exit 0
