#!/bin/bash
#####################################################################
# File name: ihammergo_barcode.sh
# Type: Shell Script
# Created on: 2019/06/29
# History:
#   v0.1
#     2019/06/29 建立文件
#####################################################################
### BEGIN INIT INFO
# Provides:          ihammergo_barcode
# Required-Start:    $all
# Required-Stop:     $all
# Default-Start:     
# Default-Stop:      0 1 6
# Short-Description: QR Code Reader
# Description:       iHammer GO! Team's QR Code Reader
### END INIT INFO
# 配置
PATH="/sbin:/bin:/usr/sbin:/usr/bin"
NAME="ihammergo_barcode.sh"
DESC="iHammer GO! Team's QR Code Reader"
LOG_FILE="/var/log/ihammer_go/ihammergo_barcode.log"
PID_DIR="/var/run/ihammer_go"

PORT=8082

# 引入 init脚本库函数
. /lib/lsb/init-functions

# PID文件目录创建
if [ ! -e "$PID_DIR" ]; then
  mkdir "$PID_DIR"
  #chown root:adm "$PID_DIR"
fi
# 日志生成
if [ ! -e "$LOG_FILE" ]; then
  LOG_DIR=${LOG_FILE%/*}
  if [ ! -e "$LOG_DIR" ]; then
    mkdir "$LOG_DIR"
    #chown root:adm "$LOG_DIR"
  fi

  touch "$LOG_FILE"
  chmod 644 "$LOG_FILE"
  # 禁用，允许快速清除日志
  #chown root:adm "$LOG_FILE"
fi

i=0
# 主程序
case "$1" in
  start|"")
    log_daemon_msg "Starting $DESC" "$NAME"

    # 不允许重复启动（单例）
    pidofproc -p "${PID_DIR}/ihammergo_barcode_reader.pid" "python3" >/dev/null
    if [ $? -eq 0 ]; then
      log_end_msg 0
      exit 0
    fi

    start-stop-daemon --start --oknodo --chuid pi \
-m --pidfile "${PID_DIR}/ihammergo_barcode_reader.pid" \
--exec /home/pi/iHammerGo/python/barcode_reader.sh & \
#>>"$LOG_FILE" 2>&1 &

    [ $? -eq 0 ] && start-stop-daemon --start --oknodo --chuid pi \
-m --pidfile "${PID_DIR}/ihammergo_barcode_stream.pid" \
--exec /usr/local/bin/mjpg_streamer -- \
-i "input_file.so -f /home/pi/iHammerGo/tmp/barcode_stream/ -d 0.5 -r" \
-o "output_http.so -p $PORT" \
>>"$LOG_FILE" 2>&1 &

    log_end_msg $?
    exit $?
    ;;

  stop)
    log_daemon_msg "Stopping $DESC" "$NAME"

    start-stop-daemon --stop --signal 2 \
--pidfile "${PID_DIR}/ihammergo_barcode_stream.pid" \
>>"$LOG_FILE" 2>&1

    start-stop-daemon --stop \
--pidfile "${PID_DIR}/ihammergo_barcode_reader.pid" \
>>"$LOG_FILE" 2>&1 && sleep 1

    log_end_msg 0
    exit 0
    ;;

  restart)
    ${0} stop
    sleep 1
    ${0} start
    ;;

  reload|force-reload)
    ${0} restart
    ;;

  status)
    status_of_proc -p "${PID_DIR}/ihammergo_barcode_reader.pid" "python3" "$DESC"
    exit $?
    ;;

  *)
    echo "Usage: $NAME {start|stop|restart|reload|status}" >&2
    exit 1
    ;;
esac
