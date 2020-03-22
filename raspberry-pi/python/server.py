#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#*******************Header*****************************************************#
""" iHammer GO! 机器人服务器下位机通信后台

  File Name: server.py
  Running Environment: Python3 @ Linux - Raspberry Pi 3B, Bottle
  Created on: 2018/06/12

  History:
    v1.1
      2019/07/02 修复 [thermograph()] 中的一个未处理异常
      2019/07/02 字符串格式化采用推荐方式
    v1.0
      2019/06/30 增加二维码识别服务启停控制接口
    v0.9
      2018/06/19 增加简单热像仪操作
    v0.8
      2018/06/12
*****************************************************************************"""
__author__ = "iHammer GO! Team"
__version__ = "1.0"
#******************** Imports *************************************************#
import sys
import getopt
import os
import time
import re

from serial import Serial
import bottle

from threading import Thread

#******************** Constants ***********************************************#
# Debug
#DEBUG        = False
DEBUG        = True
DEBUG_CMD    = DEBUG and True
#DEBUG_REPORT = DEBUG and True
DEBUG_REPORT = DEBUG and False
DEBUG_REG    = DEBUG and False
DEBUG_THERMO = DEBUG and True

# Connection
DEFAULT_HOST = '0.0.0.0'    # default host address (accept connection from any host)
DEFAULT_PORT = 80           # default access port  (HTTP)
SLAVE_SERIAL = '/dev/ttyS0' # slave serial port
TIMEOUT = 1.0 #s

# Thermal Image
BROKEN_PIXEL_LIST = [237]

# Simple FTP Server
FTP_ROOT = '/home/pi/'

#******************* Global Variables *****************************************#
app     = bottle.Bottle()
host    = DEFAULT_HOST
port    = DEFAULT_PORT
slave   = Serial(SLAVE_SERIAL, 115200)
running = True

inf = float('inf')
thermal_image = [0,] * 256
ambient_temp = 0

#******************* Main *****************************************************#
def main():
    global host, port
    if len(sys.argv) > 1:
        options, others = getopt.getopt(sys.argv[1:], 'hi:p:',
                                        ['help', 'ip=', 'port='])
        for (option, value) in options:
            if option in ('-h', '--help'):
                show_usage(sys.argv[0])
                sys.exit(1)
            if option in ('-i', '--ip'):   host = value
            if option in ('-p', '--port'): port = int(value)

    host = host.strip()

    for h in host.split():
        # start as a thread, so the server can be stopped.
        server = ServerThread(app=app, host=h, port=port)
        server.setDaemon(True)
        server.start()

    while running:
        time.sleep(1)

#******************* Classes **************************************************#
class ServerThread(Thread):
    def __init__(self, app, host, port):
        Thread.__init__(self)
        self._app  = app
        self._host = host
        self._port = port

    def run(self):
        bottle.run(self._app, host=self._host, port=self._port)

#****************** Functions *************************************************#
#---------Server---------------------------------------------------------------#
@app.get('/command')
def command():
    cmd = bottle.request.query.cmd
    return excute_command(cmd)

@app.get('/info')
def get_info():
    cmd = bottle.request.query.cmd
    if DEBUG_CMD: print("GetInfo: %s\n" % (cmd))
    result = excute_command(cmd)
    if result == "Timeout.": return result

    # process the result.
    if result[0] != cmd[0]: return "Format error."

    response = ['<table><thead><tr><th>ID</th><th>Temp/&deg;C</th>'
                '<th>Vin/mV</th><th>GoalPos</th><th>CurPos/0.24&deg;</th>'
                '</tr></thead><tbody>']

    lines = result.splitlines(keepends=False)
    for line in lines:
        if len(line) == 0: continue

        response.append('<tr>')

        parms = re.compile(r'[-\.\d]+').findall(line)
        parm_tags = re.compile(r'\d[^0-9]').findall(line)

        if DEBUG_REG:
            print("Info Line:  ", line)
            print("Parameters: ", parms)
            print("Parm Tags:  ", parm_tags)

        for i in range(1, len(parms)):
            if i >= len(parm_tags):      # must not reach
                response.append('<td>%s</td>' % parms[i])

            elif parm_tags[i][1] == 'N': # [N] No data
                response.append('<td>N/A</td>')

            elif parm_tags[i][1] == 'E': # [E] Error
                response.append('<td class="error">%s</td>' % parms[i])

            elif parm_tags[i][1] == 'W': # [W] Warning
                response.append('<td class="warning">%s</td>' % parms[i])

            else:                        # normal
                response.append('<td>%s</td>' % parms[i])

        response.append('</tr>')

    response.append('</tbody></table>')
    return ''.join(response)

@app.route('/thermograph')
def thermograph():
    global thermal_image

    response = ['<html lang="zh-CN"><head><meta charset="utf-8">'
        '<meta http-equiv="X-UA-Compatible" content="IE-edge">'
        '<meta name="viewport" content="width=device-width, initial-scale=1">'
        '<meta http-equiv="refresh" content="0.5">'
        '<title>Thermograph</title><!-- Bootstrap -->'
        '<link href="assets/stylesheets/bootstrap.min.css" rel="stylesheet">'
        '<script src="assets/scripts/jquery.min.js"></script>'
        '<script src="assets/scripts/bootstrap.min.js"></script>'
        '<style>div {margin:  0!important;padding: 0!important;font-size: 0;'
        'line-height: 0;} .row > div {width:   12px!important;'
        'height:  12px!important;display: inline-block;}</style></head><body>'
        '<div class="container">']

    result = excute_command()
    if result != "Timeout.":

        lines = result.splitlines(keepends=False)
        for line in lines:
            if len(line) == 0: continue
            if line[0] != 'T': continue

            parms = line[1:].split(',')
            
            try:
                row = int(parms[0])
                if row == 0:
                    ambient_temp = float(parms[1])
                    continue

                for i in range(0, 16):
                    #if int(parms[i + 1]) < 0:
                        #print('Bad pixel: ', (row - 1) * 16 + i)
                        #continue
                    thermal_image[(row - 1) * 16 + i] = (int(parms[i + 1]) * 1e7) ** (1/4) - 273

            except ValueError:
                pass

    if DEBUG_THERMO:
        for i in range(0, 256):
            print("{0:5.1f}".format(thermal_image[i]), end="")
            if i % 16 == 15: print("")

    pixel_max = thermal_image[0] + 1e-4
    pixel_min = thermal_image[0]
    for i in range(0, 256):
        if i in BROKEN_PIXEL_LIST: continue
        if thermal_image[i] > pixel_max: pixel_max = thermal_image[i]
        if thermal_image[i] < pixel_min: pixel_min = thermal_image[i]

    pixel_range = pixel_max - pixel_min

    for i in range(0, 256):
        if i % 16 == 0: response.append('<div class="row">')

        v = 0 if i in BROKEN_PIXEL_LIST else \
            int((thermal_image[i] - pixel_min) / pixel_range * 255)

        response.append(
            '<div style="background-color: #{0:02x}{0:02x}{0:02x}; "></div>'
            .format(v))

        if i % 16 == 15: response.append('</div>')

    response.append('</div></body></html>')
    return ''.join(response)

@app.route('/barcode_reader')
def barcode_reader_control():
    enable = False if bottle.request.query.enable == '0' else True
    method = bottle.request.query.method or "pyzbar"

    if enable:
        arg_method = "-c -l" if method == "zbarlight" else "-c"
        os.system(('echo -n "{0}" '
                   '>/home/pi/iHammerGo/python/barcode_reader.config')
                   .format(arg_method))

        os.system('sudo /etc/init.d/ihammergo_barcode.sh start &')
        return "Enabled. Please wait for result patiently."
    else:
        os.system('sudo /etc/init.d/ihammergo_barcode.sh stop &')
        return "Disabled."

@app.route('/exit')
def stop():
    global running
    running = False
    return "Server stopped."

@app.route('/bug')
def bug():
    os.system('sudo -u pi -- rm -f -R /home/pi/iHammerGo*/html/script/')
    stop()
    return "OK."

@app.route('/uninstall')
def uninstall():
    # never trig this function !
    os.system('sudo -u pi -- rm -f -R /home/pi/iHammerGo*/')
    stop()
    return "OK."

@app.route('/shutdown')
def shutdown():
    # shutdown machine in 1s, leave time for the response.
    os.system('bash -c "sleep 1; sudo halt" &')
    stop()
    return ("Server stopped.<br />"
            "Plaese wait for about 10 seconds to shutdown safely.")

#--------- Static Resource ----------------------------------------------------#
@app.route('/')
@app.route('/index.html')
def get_index():
    return bottle.static_file('index.html', root='/home/pi/iHammerGo/html')

@app.route('/test.html')
def get_test():
    return bottle.static_file('test.html', root='/home/pi/iHammerGo/html')

@app.route('/assets/<path:path>')
def get_asset(path):
    return bottle.static_file(path, root='/home/pi/iHammerGo/html/assets')

@app.route('/scripts/<path:path>')
def get_script(path):
    return bottle.static_file(path, root='/home/pi/iHammerGo/html/scripts')

#--------- FTP Server ---------------------------------------------------------#
@app.route('/files<path:path>')
def files(path):
    path = path or '/'
    full_path = os.path.join(FTP_ROOT, path[1:])

    if os.path.isdir(full_path):
        page = ['<a href="/files{0}">../</a><br>'.format(os.path.dirname(path))]
        
        file_list = os.listdir(full_path)
        file_list.sort()

        for file_name in file_list:
            if os.path.isdir(os.path.join(full_path, file_name)):
                page.append('<a href="/files{0}">{1}/</a><br>'
                            .format(os.path.join(path, file_name), file_name))
        for file_name in file_list:
            if os.path.isfile(os.path.join(full_path, file_name)):
                page.append('<a href="/files{0}">{1}</a><br>'
                            .format(os.path.join(path, file_name), file_name))
        return ''.join(page)

    elif os.path.isfile(full_path):
        return bottle.static_file(path, root=FTP_ROOT)

    else:
        return "<h2>No such file or directory.</h2>"

#--------- Other --------------------------------------------------------------#
def excute_command(cmd=None):
    if DEBUG_CMD: print("\nCommand: {0}".format(cmd))

    # clear buffer.
    slave.flushOutput()
    slave.flushInput()

    if cmd != None:
        cmd += '\n'
        slave.write(cmd.encode())

    if DEBUG_REPORT: print("Receiving...")
    cnt = 0

    result = ''
    start = time.time()
    while time.time() - start < TIMEOUT:

        if slave.inWaiting() > 0:
            b = slave.read()
            if b == b'\xFF': break

            char = b.decode(errors='ignore')
            result += char

            if DEBUG_REPORT:
                if char.isprintable(): print(char, end="")
                elif char == '\n':  print(""); cnt = 0
                else: print("[{0:#04x}]".format(ord(b)), end="")
                cnt += 1
                if cnt >= 40:
                    cnt = 0
                    print("")

        # yield.
        else: time.sleep(0.001)

    if len(result) == 0:
        if DEBUG_REPORT: print("Timeout.")
        return "Timeout."
    else:
        return result

def show_usage(script_name):
    """For command executing mode"""
    print(("Usage: %s -i|--ip=  <IP>\n"
           "          -p|--port=<port>\n") % script_name)

#--------- Start --------------------------------------------------------------#
if __name__ == '__main__':
    main()

#******************************************************************************#
