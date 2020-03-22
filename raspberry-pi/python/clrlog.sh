#!/bin/bash
LOG="/home/pi/iHammerGo/python/server.log"

echo "Log:"
tail -20 ${LOG}

echo -n "" >${LOG}
echo
echo "Cleared."

echo
exit 0
