/*******************************************************************************
 * File name: CommandReceiver.h
 * Source file: CommandReceiver.cpp
 * Created on: 2018年2月2日
 * Author: Wuxy
 * 基于字符的串口指令接收机 Simple Command Receiver char-based
*******************************************************************************/
#ifndef COMMANDRECERVER_H_
#define COMMANDRECERVER_H_

#include "Arduino.h"

#include "BlockStream.h"

#define CMD_BUFFER_LENGTH 32

// for example, a command "M100, 234, -567"
// 'M' is the command head, only 1 character.
#define CMD_FIELD_HEAD ((uint8_t)0)
// '100' is the 1st parameter, and '234' is 2nd, and so on.
#define CMD_FIELD_PARM(_parmn) ((uint8_t)(_parmn))

class CommandReceiver : public BlockStream {
	private:
		const char *mHeaders;
		const char mDelimiter;
		
	protected:
		size_t predictSize(uint8_t *blockPart, size_t size);
		// we needn't check sum, but we use it to free new line characters.
		bool checkSum(uint8_t *block, size_t size);
		
	public:
		// attach CommandReceiver to a Stream.
		// we need a table of command head, used a string.
		// 'delimiter' is used for split parameters.
		CommandReceiver(Stream &bus, const char *headers, const char delimiter);
		
		// dynamic read. [See CMD_FIELD_* at the top of the file]
		uint32_t read(uint8_t field);
		// read the head of command.
		uint8_t readHead();
		// read command parameters.
		uint32_t readParm(uint8_t index);
		// read whole command in characters.
		using BlockStream::read;
};

#endif /* COMMANDRECERVER_H_ */
