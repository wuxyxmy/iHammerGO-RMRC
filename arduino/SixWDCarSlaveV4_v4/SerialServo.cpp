/*******************************************************************************
 * File name: SerialServo.cpp
 * Header file: SerialServo.h
 * Rely on: SerialServoBus.h/cpp
 * Created on: 2018年2月2日
 * Author: Wuxy
 * 串行舵机库
 *
 * History:
 *  v21
 *    2018/06/09 修正关于
 *    2018/06/02 增加attach()方法，修改mBus为SerialServoBus*类型
*******************************************************************************/
#include "SerialServo.h"

/*******************File Public************************************************/
// Tx数据包：帧头(2) {ID(1) 长度(1) 指令(1) 参数(0~4)} 校验(1)
// (字节数 Bytes) {有效载荷}
// Commands             Cmd_Number, {Length}, {(Length_RxFrame)}
#define SERVO_WRITE_MOVE_CMD       1 , 7
#define SERVO_READ_MOVE_CMD        2 , 3, 7
#define SERVO_WRITE_NEXT_MOVE_CMD  7 , 7
#define SERVO_READ_NEXT_MOVE_CMD   8 , 3, 7
#define SERVO_MOVE_START           11, 3
#define SERVO_MOVE_STOP            12, 3
#define SERVO_WRITE_ID             13, 4
#define SERVO_READ_ID              14, 3, 4
#define SERVO_ADJUST_ANGLE_OFFSET  17, 4
#define SERVO_WRITE_ANGLE_OFFSET   18, 3
#define SERVO_READ_ANGLE_OFFSET    19, 3, 4
#define SERVO_WRITE_ANGLE_LIMIT    20, 7
#define SERVO_READ_ANGLE_LIMIT     21, 3, 7
#define SERVO_WRITE_VIN_LIMIT      22, 7
#define SERVO_READ_VIN_LIMIT       23, 3, 7
#define SERVO_WRITE_TEMP_LIMIT     24, 4
#define SERVO_READ_TEMP_LIMIT      25, 3, 4
#define SERVO_READ_TEMP            26, 3, 4
#define SERVO_READ_VIN             27, 3, 5
#define SERVO_READ_POS             28, 3, 5
#define SERVO_WRITE_MODE           29, 7
#define SERVO_READ_MODE            30, 3, 7
#define SERVO_WRITE_POWER          31, 4
#define SERVO_READ_POWER           32, 3, 4
#define SERVO_WRITE_LED_CTRL       33, 4
#define SERVO_READ_LED_CTRL        34, 3, 4
#define SERVO_WRITE_LED_ERROR      35, 4
#define SERVO_READ_LED_ERROR       36, 3, 4

// Servo Running Mode
#define SERVO_MODE_MOTOR 0x01
#define SERVO_MODE_SERVO 0x00

/*******************File Private***********************************************/
// Help Macros for [Commands]
#define _SERVO_LENGTH(cmd, tx_length, ...) ((tx_length) + 3)
#define SERVO_LENGTH(cmd) _SERVO_LENGTH(cmd)

#define _SERVO_RX_LENGTH(cmd, tx_length, rx_length) ((rx_length) + 3)
#define SERVO_RX_LENGTH(cmd) _SERVO_RX_LENGTH(cmd)

#define _SERVO_RX_INFO(cmd, tx_length, rx_length) cmd, rx_length
#define SERVO_RX_INFO(cmd) _SERVO_RX_INFO(cmd)

/*******************Class Public***********************************************/
//--------Constructers--------------------------------------------------------//
SerialServo::SerialServo() :
  mBus(NULL),
  mId(INVALID_ID),
  // 强制在第一次调用[SerialServo::writeRaw()]时将模式设置为“舵机模式”
  mMode(SERVO_MODE_MOTOR)
{
}

SerialServo::SerialServo(SerialServoBus &bus, uint8_t id) :
	mBus(&bus),
	mId(id),
	// 强制在第一次调用[SerialServo::writeRaw()]时将模式设置为“舵机模式”
	mMode(SERVO_MODE_MOTOR)
{
}

//--------Common Function-----------------------------------------------------//
void SerialServo::attach(SerialServoBus &bus, uint8_t id) {
  mBus = &bus;
  mId = id;
  // 强制在第一次调用[SerialServo::writeRaw()]时将模式设置为“舵机模式”
  mMode = SERVO_MODE_MOTOR;
}

int8_t SerialServo::changeID(uint8_t newID) {
	if (newID > MAX_ID) return FAIL;
	
	if (mBus->send(mId, SERVO_WRITE_ID) == SERVO_LENGTH(SERVO_WRITE_ID)) {
		mId = newID;
		return PASS;
	} else {
		return FAIL;
	}
}

int8_t SerialServo::write(uint8_t degress, uint16_t time) {
	if (degress <= 240)
		return writeRaw(map(degress, 0, 240, 0, 1000), time);
	else
		return writeRaw(1000, time);
}

int8_t SerialServo::writeRaw(uint16_t position, uint16_t time) {
	// 如果不使用该方法切换模式，舵机将不能正常运行
	if (mMode != SERVO_MODE_SERVO) {
		mMode = SERVO_MODE_SERVO;
		if (mBus->send(mId, SERVO_WRITE_MODE, SERVO_MODE_SERVO, 0) !=
			SERVO_LENGTH(SERVO_WRITE_MODE)) return FAIL;
	}
	// force 'position' between 0 and 1000.
	if (position > 1000) position = 1000;

  return mBus->send(mId, SERVO_WRITE_MOVE_CMD, position, time) ==
    SERVO_LENGTH(SERVO_WRITE_MOVE_CMD) ? PASS : FAIL;
}

int8_t SerialServo::stop() {
	if (mMode == SERVO_MODE_SERVO) {
		return mBus->send(mId, SERVO_MOVE_STOP) ==
			SERVO_LENGTH(SERVO_MOVE_STOP) ? PASS : FAIL;
	} else {
		return mBus->send(mId, SERVO_WRITE_MODE, SERVO_MODE_MOTOR, 0) ==
			SERVO_LENGTH(SERVO_WRITE_MODE) ? PASS : FAIL;
	}
}

int8_t SerialServo::run(int16_t speed) {
	// 更改模式标志 (See [SerialServo::writeRaw()])
	mMode = SERVO_MODE_MOTOR;
	
	return mBus->send(mId, SERVO_WRITE_MODE, SERVO_MODE_MOTOR,
		(uint16_t)speed) == SERVO_LENGTH(SERVO_WRITE_MODE) ? PASS : FAIL;
}

int8_t SerialServo::powerOn() {
	return mBus->send(mId, SERVO_WRITE_POWER, 1) ==
		SERVO_LENGTH(SERVO_WRITE_POWER) ? PASS : FAIL;
}

int8_t SerialServo::powerOff() {
	return mBus->send(mId, SERVO_WRITE_POWER, 0) ==
		SERVO_LENGTH(SERVO_WRITE_POWER) ? PASS : FAIL;
}

int16_t SerialServo::read() {
	if (mBus->send(mId, SERVO_READ_POS) != SERVO_LENGTH(SERVO_READ_POS)) {
		return FAIL_VALUE;
	}
	
	uint8_t *frame;
	// if we are too strict, it will not give us anything.
	// TODO could we do it better?
	if(mBus->timedPeek(frame, mId, SERVO_RX_INFO(SERVO_READ_POS)) == 0) {
    mBus->read();
		return FAIL_VALUE;
	}
	
  mBus->read();
	return word(frame[SERVO_FRAME_PARM2], frame[SERVO_FRAME_PARM1]);
}

// 读取电压值(毫伏)
int16_t SerialServo::readVoltage() {
	if (mBus->send(mId, SERVO_READ_VIN) != SERVO_LENGTH(SERVO_READ_VIN)) {
		return FAIL_VALUE;
	}
	
	uint8_t *frame;
	if(mBus->timedPeek(frame, mId, SERVO_RX_INFO(SERVO_READ_VIN)) == 0) {
    mBus->read();
		return FAIL_VALUE;
	}
	
  mBus->read();
	return word(frame[SERVO_FRAME_PARM2], frame[SERVO_FRAME_PARM1]);
}

int16_t SerialServo::readTemperature() {
	if (mBus->send(mId, SERVO_READ_TEMP) != SERVO_LENGTH(SERVO_READ_TEMP)) {
		return FAIL_VALUE;
	}
	
	uint8_t *frame;
	if(mBus->timedPeek(frame, mId, SERVO_RX_INFO(SERVO_READ_TEMP)) == 0) {
	  mBus->read();
		return FAIL_VALUE;
	}
	
  mBus->read();
	return frame[SERVO_FRAME_PARM1];
}
/******************************************************************************/
