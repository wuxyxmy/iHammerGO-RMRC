#!/bin/bash
#####################################################################
# File name: ihammergo_video.sh
# Type: Shell Script
# Created on: 2018/06/17
# History:
#   v0.9
#     2019/06/26 文件名由 'video_stream.sh' 修改 
#     2019/06/25 增加分辨率设置
#####################################################################
### BEGIN INIT INFO
# Provides:          ihammergo_video
# Required-Start:    $all
# Required-Stop:     $all
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: iHammer GO! Team's Video Stream
# Description:       Provides several video streamers for remote control.
### END INIT INFO
# 配置
PATH="/sbin:/bin:/usr/sbin:/usr/bin"
NAME="ihammergo_video.sh"
DESC="iHammer GO!'s Video Stream"
LOG_FILE="/var/log/ihammer_go/ihammergo_video.log"
PID_DIR="/var/run/ihammer_go"

DEVICES=("/dev/video0" "/dev/video1")
PORTS=(8080 8081)
# 由于设备编号可能在重新启动后改变，使用所有设备的最高分辨率
RESOLUTION=("1280x720" "1280x720")

LOG_DIR=${LOG_FILE%/*}

# 引入 init脚本库函数
. /lib/lsb/init-functions

# 检查配置
if [ ${#DEVICES[@]} != ${#PORTS[@]} ] || \
   [ ${#DEVICES[@]} != ${#RESOLUTION[@]} ] || \
   [ ${#PORTS[@]}   != ${#RESOLUTION[@]} ]
then
  echo "Configuration error." >&2
  exit 1
fi

# PID文件目录创建
if [ ! -e "$PID_DIR" ]; then
  mkdir "$PID_DIR"
  #chown root:adm "$PID_DIR"
fi
# 日志生成
if [ ! -e "$LOG_FILE" ]; then
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
    
    i=0
    while [ $i -lt ${#DEVICES[@]} ]; do

      start-stop-daemon --start --oknodo \
-m --pidfile "${PID_DIR}/ihammergo_video_${i}.pid" \
--exec /usr/local/bin/mjpg_streamer -- \
-i "input_uvc.so -d ${DEVICES[i]} --resolution ${RESOLUTION[i]}" \
-o "output_http.so --port ${PORTS[i]}" \
>>"$LOG_FILE" 2>&1 &

      let i++
    done

    # SERVER
    # its log is forwarding to a separated file.
    start-stop-daemon --start --oknodo \
-m --pidfile "${PID_DIR}/ihammergo_video_server.pid" \
--exec /home/pi/iHammerGo/python/server.sh \
>>"$LOG_FILE" 2>&1 &

    log_end_msg 0
    ;;

  stop)
    log_daemon_msg "Stopping $DESC" "$NAME"

    start-stop-daemon --stop \
--pidfile "${PID_DIR}/ihammergo_video_server.pid" \
>>"$LOG_FILE" 2>&1 || sleep 1

    i=0
    while [ $i -lt ${#DEVICES[@]} ]; do

      # "&&"：在有进程时执行 "sleep 1"
      start-stop-daemon --stop --signal 2 \
--pidfile "${PID_DIR}/ihammergo_video_${i}.pid" \
>>"$LOG_FILE" 2>&1 || sleep 1

      let i++
    done

    # 等待进程停止
    #sleep 2
    log_end_msg 0
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
    status_of_proc -p "${PID_DIR}/ihammergo_video_0.pid" "mjpg_streamer" "$DESC"
    ;;

  *)
    echo "Usage: $NAME {start|stop|restart|reload|status}" >&2
    exit 1
    ;;
esac

exit 0
