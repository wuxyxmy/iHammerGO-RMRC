/*******************************************************************************
 * File name: SerialServoBus.cpp
 * Header file: SerialServoBus.h
 * Rely on: BlockStream.h/cpp
 * Created on: 2018年2月2日
 * Author: Wuxy
 * 串行舵机接口/总线
*******************************************************************************/
#include "SerialServoBus.h"

/*******************File Private***********************************************/
// Debug
//#define DEBUG

// Tx数据包：帧头(2) {ID(1) 长度(1) 指令(1) 参数(0~4)} 校验(1)
// (字节数) {有效载荷}
static uint8_t checkSum(uint8_t *frame) {
	uint16_t sum = 0;
	uint8_t i;
	// 2: Sees [checkSum(uint8_t *frame)]
	for(i = SERVO_FRAME_ID; i < frame[SERVO_FRAME_LENGTH] + 2; i ++)
		sum += frame[i];
	frame[i] = (uint8_t)~sum;
	return frame[i];
}

/*******************Class Public***********************************************/
//--------Constructors--------------------------------------------------------//
SerialServoBus::SerialServoBus(Stream &bus, uint32_t timeout) :
	BlockStream(bus, SERVO_FRAME_HEADER_ARRAY, 10, DEFAULT_BUFFER_SIZE,
	timeout)
{
}

//--------Implementations-----------------------------------------------------//
size_t SerialServoBus::predictSize(uint8_t* frame, size_t size) {
	if (size > SERVO_FRAME_LENGTH) {
		// 3: See [checkSum(uint8_t *frame)].
		return frame[SERVO_FRAME_LENGTH] + 3;
	} else {
		return 0;
	}
}

bool SerialServoBus::checkSum(uint8_t* frame, size_t /*size*/) {
	// although 'checkSum' will changes the checksum filed, we read it
	// first.
	// 2: See [checkSum(uint8_t *frame)].
	return frame[frame[SERVO_FRAME_LENGTH] + 2] == ::checkSum(frame);
}

//--------Member Function-----------------------------------------------------//
// 发送长度为3(一般为读指令)或4字节的指令
size_t SerialServoBus::send(uint8_t id, uint8_t cmd, uint8_t length,
	uint8_t parm)
{
	uint8_t frame[length + 3];
	frame[SERVO_FRAME_HADER1] = SERVO_FRAME_HEADER;
	frame[SERVO_FRAME_HADER2] = SERVO_FRAME_HEADER;
	frame[SERVO_FRAME_ID] = id;
	frame[SERVO_FRAME_LENGTH] = length;
	frame[SERVO_FRAME_CMD] = cmd;
	// we just writes 0 to checksum filed, if the 'parm' is ignore,
	// because length of the frame is depend on 'length'.
	frame[SERVO_FRAME_PARM1] = parm;
	// also, we never skip sum checking.
	::checkSum(frame);
	return write(frame, length + 3);
}

// 执行长度为7字节的指令(一般为写指令)
size_t SerialServoBus::send(uint8_t id, uint8_t cmd, uint8_t length,
	uint16_t parm12, uint16_t parm34)
{
	uint8_t frame[length + 3];
	frame[SERVO_FRAME_HADER1] = SERVO_FRAME_HEADER;
	frame[SERVO_FRAME_HADER2] = SERVO_FRAME_HEADER;
	frame[SERVO_FRAME_ID] = id;
	frame[SERVO_FRAME_LENGTH] = length;
	frame[SERVO_FRAME_CMD] = cmd;
	frame[SERVO_FRAME_PARM1] = lowByte(parm12);
	frame[SERVO_FRAME_PARM2] = highByte(parm12);
	frame[SERVO_FRAME_PARM3] = lowByte(parm34);
	frame[SERVO_FRAME_PARM4] = highByte(parm34);
	::checkSum(frame);
	return write(frame, length + 3);
}

size_t SerialServoBus::read(uint8_t *dst, uint8_t id, uint8_t cmd,
	uint8_t rxLength)
{
	uint8_t *nothing;
	if (peek(nothing, id, cmd, rxLength) > 0)
		// find it. (peek() keeps the match frame on the buffer tail.)
		return read(dst);
	nothing = NULL;
	return 0;
}

size_t SerialServoBus::peek(uint8_t* &dst, uint8_t id, uint8_t cmd,
	uint8_t rxLength)
{
	uint8_t *frame;
	size_t size = peek(frame);
	for (uint8_t i = 0; i < available(); i ++) {
		if (size == 0) return 0;

		if (frame[SERVO_FRAME_ID] == id && frame[SERVO_FRAME_CMD] == cmd
			&& frame[SERVO_FRAME_LENGTH] == rxLength)
		{
			// we find the correct frame.
			dst = frame;
			return size;
		}
		
		// put the last frame at the buffer head.
		// does it allow multi-thread ?
		//uint8_t oldSREG = SREG;
  	//cli();

		uint8_t oldHead = mRxBuffer_Head;
    mRxBuffer_Head = (mRxBuffer_Head + 1) % mBufferSize;
		mRxBuffer[mRxBuffer_Head].block = mRxBuffer[oldHead].block;
		mRxBuffer[mRxBuffer_Head].size = mRxBuffer[oldHead].size;
		mRxBuffer[oldHead].block = mRxBuffer[mRxBuffer_Tail].block;
		mRxBuffer[oldHead].size = mRxBuffer[mRxBuffer_Tail].size;
		mRxBuffer[mRxBuffer_Tail].block = NULL;
		mRxBuffer[mRxBuffer_Tail].size = 0;
    mRxBuffer_Tail = (mRxBuffer_Tail + 1) % mBufferSize;

		//SREG = oldSREG;
		
		size = peek(frame);
	}
	
	// there isn't any match frame.
	return 0;
}

size_t SerialServoBus::timedRead(uint8_t *dst, uint8_t id, uint8_t cmd,
    uint8_t rxLength) {
  size_t size = 0;
  mStartMillis = millis();
  do {
    size = read(dst, id, cmd, rxLength);
    if (size > 0) return size;

    // you can use listener() in yield() to run like a multi-thread system.
    if (serialEventRun) serialEventRun();
    yield();
  } while(millis() - mStartMillis < mTimeout);

  // timeout.
  return 0;
}

size_t SerialServoBus::timedPeek(uint8_t* &dst, uint8_t id, uint8_t cmd,
    uint8_t rxLength) {
  size_t size = 0;
  mStartMillis = millis();
  do {
    size = peek(dst, id, cmd, rxLength);
    if (size > 0) return size;

    // you can use listener() in yield() to run like a multi-thread system.
    if (serialEventRun) serialEventRun();
    yield();
  } while(millis() - mStartMillis < mTimeout);

  // timeout.
  return 0;
}

void SerialServoBus::clear() {
#ifdef DEBUG
	for(uint8_t p=0;p<mBufferSize;p++) {
	  Serial.println();
		Serial.print(mRxBuffer[p].size,DEC);
		Serial.print('\t');
		for(uint8_t i=0;i<mRxBuffer[p].size;i++) {
			if(mRxBuffer[p].block[i]<16) Serial.print('0');
			Serial.print(mRxBuffer[p].block[i],HEX);
		}
		if(p==mRxBuffer_Head)	Serial.print(F("\t<-H"));
		if(p==mRxBuffer_Tail)	Serial.print(F("\t<-T"));
	}
  Serial.println();
#endif

	while (available() > 0) read();
}
/******************************************************************************/
