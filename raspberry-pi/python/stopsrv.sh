#!/bin/bash
PIDS=$(ps -eo pid:1,comm | grep python | cut -d " " -f 1)

if [ ! -z "${PIDS}" ]; then
  echo "Running process:"

  ps -eo pid,cmd | grep -e \
$(echo ${PIDS} | cut -d " " -f 1- --output-delimiter "\\|" -) \
| grep python \
#| head -n -1 -

  echo
  sudo kill -s SIGKILL ${PIDS}

  echo "Done."

else

  echo "No process found."

fi

exit 0
