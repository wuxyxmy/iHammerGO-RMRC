#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#****************** Header ****************************************************#
""" iHammer GO! 机器人服务器二维码识别服务后台

  File Name: barcode_reader.py
  Environment: Python3 @ Linux - Raspberry Pi 3B, Bottle, Zbarlight, OpenCV 3.4.0
  Created on: 2019/06/23

  History:
    v0.2
      2019/06/30 输出重定向，日志立即写入文件
      2019/06/29 适配作为后台服务启动的要求
    v0.1
      2019/06/29 文件名由 'qr_code_reader.py' 修改
      2019/06/23 代码整理
"""
__author__ = "iHammer GO! Team"
__version__ = "0.2"
#******************** Imports *************************************************#
import sys
import getopt
import os
import time

import cv2
import pyzbar.pyzbar as pyzbar

import zbarlight
import PIL

#******************** Constants ***********************************************#
# Debug
#DEBUG        = False
DEBUG        = True
DEBUG_IMAGE  = DEBUG and False
DEBUG_TIME   = DEBUG and True
DEBUG_OUTPUT = DEBUG and True

DEFAULT_PORT = 8080
DEFAULT_FILE = 'http://localhost:8080/?action=stream&ignored.mjpg'
DEFAULT_OUTPUT_DIR = '/home/pi/iHammerGo/tmp/barcode_stream/'

LOG_FILE = '/home/pi/iHammerGo/python/barcode_reader.log'

#******************* Global Variables *****************************************#
port = DEFAULT_PORT
file = DEFAULT_FILE
out_dir = DEFAULT_OUTPUT_DIR
continuously  = False
use_zbarlight = False

frame_cnt = 0

log = None

#******************* Main *****************************************************#
def main():
    global port, file, continuously, use_zbarlight, out_dir, capture, frame_cnt, log

    if len(sys.argv) > 1:
        options, others = getopt.getopt(sys.argv[1:],
                                        'cf:hlo',
                                        ['continuously', 'file=', 'help', 'zbarlight', 'output-dir'])
        for (option, value) in options:
            if option in ('-h', '--help'):
                show_usage(sys.argv[0])
                sys.exit(1)
            if option in ('-f', '--file'):         file = value.strip()
            if option in ('-c', '--continuously'): continuously = True
            if option in ('-l', '--zbarlight'):    use_zbarlight = True
            if option in ('-o', '--output-dir'):   out_dir = value.strip()

    try:
        log = open(LOG_FILE, 'a')
    except:
        print("Can't open log file ('{0}')!".format(LOG_FILE), file=sys.stderr)
        log.close()
        sys.exit(1)

    if not os.path.isdir(out_dir):
        print("'{0}' is not a existing directory!".format(out_dir), file=sys.stderr)
        sys.exit(1)

    capture = cv2.VideoCapture(file)
    if not capture.isOpened():
        print("Failed to open '{0}'!".format(file), file=sys.stderr)
        capture.release()
        sys.exit(1)

    if continuously:
        while True:
            start = time.time()
            for i in range(8): capture.read() # skip frames
            if DEBUG_TIME: print("[DEBUG] Skipping {0:.3f}s".format(time.time() - start))

            result, frame = capture.read()
            if len(frame) == 0:
                print("[ WARN] Frame lost!")
                continue

            decode_and_mark(frame)
            cv2.imwrite(os.path.join(out_dir, 'frame_{0:03d}.jpg'.format(frame_cnt)), frame)
            frame_cnt += 1
            if frame_cnt >= 1000: frame_cnt = 0

            if DEBUG_TIME: print("[DEBUG] Totally cost {0:.3f}s".format(time.time() - start))

            if DEBUG_IMAGE:
                cv2.imshow("Video", frame)
                # run until <Esc> press
                if cv2.waitKey(50) == 27: break

            else: time.sleep(0.05)

    else:
        result, frame = capture.read()
        if len(frame) == 0:
            print("[ WARN] Frame lost!")
        else:
            decode_and_mark(frame)
            cv2.imwrite(os.path.join(out_dir, 'frame_00.jpg'), frame)
            
            if DEBUG_IMAGE:
                cv2.imshow("Snapshot", frame)
                while cv2.waitKey() != 27: pass

    capture.release()
    cv2.destroyAllWindows()
    log.close()

#****************** Functions *************************************************#
def decode_and_mark(frame):
    if use_zbarlight: frame = decode_via_zbarlight(frame)
    else:             frame = decode_via_pyzbar(frame)
    return frame

def decode_via_pyzbar(image):
    start_time = time.time()
    barcodes = pyzbar.decode(cv2.cvtColor(image, cv2.COLOR_BGR2GRAY), symbols=[pyzbar.ZBarSymbol.QRCODE])
    time_cost = time.time() - start_time

    if DEBUG_TIME: print("[DEBUG] Processing cost {:.3f}s via pyzbar".format(time_cost))

    for barcode in barcodes:
        # 提取条形码的边界框的位置
        # 画出图像中条形码的边界框
        (x, y, w, h) = barcode.rect
        cv2.rectangle(image, (x, y), (x + w, y + h), (0, 0, 255), 2)
        # 条形码数据为字节对象，所以如果我们想在输出图像上
        # 画出来，就需要先将它转换成字符串
        barcodeData = barcode.data.decode("utf-8")
        barcodeType = barcode.type
        # 绘出图像上条形码的数据和条形码类型
        text = "%s (%s)" % (barcodeData, barcodeType)
        cv2.putText(image, text, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX,
                    .5, (0, 0, 125), 2)
        # 向终端打印条形码数据和条形码类型
        print("[{0:5.2f}] Found {1}: {2}".format(time.time(), barcodeType, barcodeData), file=log, flush=True)
        if DEBUG_OUTPUT: print("[{0:5.2f}] Found {1}: {2}".format(time.time(), barcodeType, barcodeData), flush=True)

    else:
        print("[{0:5.2f}] No barcodes found".format(time.time()), file=log, flush=True)
        if DEBUG_OUTPUT: print("[{0:5.2f}] No barcodes found".format(time.time()), flush=True)

    return image

def decode_via_zbarlight(image):
    start_time = time.time()
    image_pil = PIL.Image.fromarray(cv2.cvtColor(image, cv2.COLOR_BGR2GRAY))
    barcodes = zbarlight.scan_codes(['qrcode'], image_pil)
    time_cost = time.time() - start_time

    if DEBUG_TIME: print("[DEBUG] Processing cost {0:.3f}s via zbarlight".format(time_cost))
    
    if barcodes == None:
        print("[{0:5.2f}] No barcodes found".format(time.time()), file=log, flush=True)
        if DEBUG_OUTPUT: print("[{0:5.2f}] No barcodes found".format(time.time()), flush=True)
    else:
        for barcode in barcodes:
            print("[{0:5.2f}] Found QRCODE: {1}".format(time.time(), barcode.decode()), file=log, flush=True)
            if DEBUG_OUTPUT: print("[{0:5.2f}] Found QRCODE: {1}".format(time.time(), barcode.decode()), flush=True)

    return image

#--------- Start --------------------------------------------------------------#
if __name__ == '__main__':
    main()

#******************************************************************************#
