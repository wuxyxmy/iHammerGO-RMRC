/*******************************************************************************
 * File name: CommandReceiver.cpp
 * Header file: CommandReceiver.h
 * Created on: 2018年2月2日
 * Author: Wuxy
 * 基于字符的串口指令接收机 Simple Command Receiver char-based
*******************************************************************************/
#include "CommandReceiver.h"

/*******************File Private***********************************************/
// Help Macros
#define isCommandEnd(_c) ((_c) == '\r' || (_c) == '\n')

/*******************Class Public***********************************************/
//--------Constructers------------- -------------------------------------------//
CommandReceiver::CommandReceiver(
  Stream &bus, const char *headers, const char delimiter) :
	BlockStream(bus, NULL, CMD_BUFFER_LENGTH, 1), // 1: only store one command.
	mHeaders(headers), mDelimiter(delimiter)
{
}

//--------Implementations-----------------------------------------------------//
size_t CommandReceiver::predictSize(uint8_t *blockPart, size_t size) {
  uint8_t &tail = blockPart[size - 1];
	if (isCommandEnd(tail)) {
		// terminate the string.
		tail = '\0';
		// stop receiving.
		return size;
	}
	// the character is not a header, stop receiving.
	/* ** NOTICE **
	 * 'strchr' also finds '\0' and return its position.
	 * example:
	 *    char s[] = "test";
   *    char *p = strchr(s, '\0');
	 *    // s --> {'t', 'e', 's', 't', '\0'}
	 *    // p --> s + 4 (address of '\0'), but not NULL!
	 */
	else if (size == 1 && strchr(mHeaders, tail) == NULL) {
	  tail = '\0';
		return 1;
	}
	// continue receiving.
	else {
	  return 0;
	}
}

bool CommandReceiver::checkSum(uint8_t *block, size_t /*size*/) {
	// consume the block of new line character and not empty (only
  // contain a '\0').
	return block[0] != '\0';
}

//--------Common Function-----------------------------------------------------//
uint32_t CommandReceiver::read(uint8_t field) {
	if (field == CMD_FIELD_HEAD) return readHead();
	else return readParm(field);
}

uint8_t CommandReceiver::readHead() {
	if (!available()) return -1;

  uint8_t *buf;
  // the block has been a string, we need't to get its length.
  peek(buf);
  // return the position of head character in mHeaders
  // we make sure that we can find it. [See listener()]
  return strchr(mHeaders, buf[0]) - mHeaders;
}

// TODO: add multiple parameter read.
uint32_t CommandReceiver::readParm(uint8_t index) {
	// we don't have got any command.
	if (!available()) return 0;

	char *start, *end;
	uint8_t count = 0;
	
	uint8_t *buf;
	peek(buf);
	// find the index parameter in the string.
	// we start finding from the 2nd character. (the 1st is the head)
	for (start = (char *)buf + 1; ++count < index; start = end + 1) {
		end = strchr(start, mDelimiter);
		// invalid index, because the delimiter isn't found.
		if (end == NULL) return 0;
	}
	
	return strtol(start, NULL, 10);
}

/******************************************************************************/
