/*******************************************************************************
 * File name: SerialServo.h
 * Source file: SerialServo.cpp
 * Rely on: SerialServoBus.h/cpp
 * Created on: 2018年2月2日
 * Author: Wuxy
 * 串行舵机库
 *
 * History:
 *  v21
 *    2018/06/02 增加attach()方法，修改mBus为SerialServoBus*类型
 *******************************************************************************/
#ifndef SERIALSERVO_H_
#define SERIALSERVO_H_

#include <Arduino.h>

#include "SerialServoBus.h"

class SerialServo {
  public:
    static const uint8_t MAX_ID       = 254;
    static const uint8_t BOARDCAST_ID = 254;
    static const uint8_t INVALID_ID   = 255;

    static const int8_t  PASS       = 0;
    // '-32640' cast to int8_t is -128 too.
    // -32640 = -32768 & -128.
    // 0x8080 = 0x8000 & 0x80.
    // -32768 (0x8000): 16 Bytes integer minimum value.
    //   -128   (0x80):  8 Bytes integer minimum value.
    static const int8_t  FAIL       = -128;   // 0x80
    static const int16_t FAIL_VALUE = -32640; // 0x8080

  protected:
    SerialServoBus *mBus;
    uint8_t mId;
    bool mMode;

  public:
    SerialServo();
    SerialServo(SerialServoBus &bus, uint8_t id);
    void attach(SerialServoBus &bus, uint8_t id);
    int8_t changeID(uint8_t newID);

    int8_t write(uint8_t degress, uint16_t time = 0);
    int8_t writeRaw(uint16_t value, uint16_t time);
    int8_t stop();
    int8_t run(int16_t speed);

    int8_t powerOn();
    int8_t powerOff();

    int16_t read();
    int16_t readVoltage();
    int16_t readTemperature();
};

#endif /* SERIALSERVO_H_ */
