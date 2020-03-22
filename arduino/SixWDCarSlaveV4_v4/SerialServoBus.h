/*******************************************************************************
 * File Name: SerialServoBus.h
 * Source File: SerialServoBus.cpp
 * Rely on: BlockStream.h/cpp
 * Created on: 2018年2月2日
 * Author: Wuxy
 * 串行舵机接口/总线
*******************************************************************************/
#ifndef SerialServoBus_h
#define SerialServoBus_h

#include <Arduino.h>

#include "BlockStream.h"

// Data Frame
#define SERVO_FRAME_HEADER_ARRAY ((uint8_t*)"\x55\x55")
#define SERVO_FRAME_HEADER 0x55
#define SERVO_FRAME_HADER1 0
#define SERVO_FRAME_HADER2 1
#define SERVO_FRAME_ID     2
#define SERVO_FRAME_LENGTH 3
#define SERVO_FRAME_CMD    4
#define SERVO_FRAME_PARM1	 5
#define SERVO_FRAME_PARM2	 6
#define SERVO_FRAME_PARM3	 7
#define SERVO_FRAME_PARM4	 8

class SerialServoBus : public BlockStream {
	protected:		
		//for [size_t timedRead(char *dst = NULL)].
		using BlockStream::timedRead;
		//for [size_t timedPeek(char* &dst)].
		using BlockStream::timedPeek;

		size_t predictSize(uint8_t* frame, size_t size);
		bool checkSum(uint8_t* frame, size_t size);
		
	public:
		SerialServoBus(Stream &bus, uint32_t timeout = 500);
		size_t send(uint8_t id, uint8_t cmd, uint8_t length, uint8_t parm = 0);
		size_t send(uint8_t id, uint8_t cmd, uint8_t length, uint16_t parm12, uint16_t parm34);
		size_t read(uint8_t *dst,  uint8_t id, uint8_t cmd, uint8_t rxLength);
		size_t peek(uint8_t* &dst, uint8_t id, uint8_t cmd, uint8_t rxLength);
		size_t timedRead(uint8_t *dst,  uint8_t id, uint8_t cmd, uint8_t rxLength);
    size_t timedPeek(uint8_t* &dst, uint8_t id, uint8_t cmd, uint8_t rxLength);
		void clear();
		
		//for [size_t read(char *dst = NULL)].
		using BlockStream::read;
		//for [size_t peek(char* &dst)].
		using BlockStream::peek;
};

#endif /* SerialServo_h */
