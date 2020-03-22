#!/bin/bash
NAME="${0##*/}"

SERVICE="/etc/init.d/ihammergo_video.sh"

show_status () {
  echo "Status:"
  sudo ${SERVICE} status
  echo
}

show_usage () {
  echo "Usage: ${NAME} {r|restart}"
  echo "       ${NAME} s|start"
  echo "       ${NAME} k|stop"
}

case "${1}" in
  "s"|"start")
    show_status
    echo "Starting..."
    sudo ${SERVICE} start
    echo "Finish."
    ;;

  "k"|"stop")
    show_status
    echo "Stopping..."
    sudo ${SERVICE} stop
    echo "Finish."
    ;;

  ""|"r"|"restart")
    show_status
    echo "Restarting..."
    sudo ${SERVICE} restart
    echo "Finish."
    ;;

  "--help")
    show_usage
    ;;

  *)
    show_usage >&2
    exit 1
    ;;
esac

echo
exit 0
