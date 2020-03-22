/*******************************************************************************
 * File name: BlockStream.h
 * Source file: BlockStream.cpp
 * Created on: 2018年2月2日
 * Author: Wuxy
 * 数据块流
*******************************************************************************/
#ifndef BLOCKSTREAM_H_
#define BLOCKSTREAM_H_

#include <Arduino.h>

#define DEFAULT_BUFFER_SIZE 8

class BlockStream {
  public:
    enum ErrorState {
      NO_ERRORS,              // no errors.
      ERROR_BAD_CHECKING_SUM, // check sum error.
      ERROR_RX_BUFFER_FULL    // receiving buffer is full.
    };

  private:
    ErrorState mErrorState;

  protected:
		Stream &mBus;

		const size_t mBlockSize;
		const uint8_t *mHeaders;

		typedef struct {
		  size_t size;
		  uint8_t *block;
		} BlockNode, *BlockNodes;

		const uint8_t mBufferSize;
		const BlockNodes mRxBuffer;
		volatile uint8_t mRxBuffer_Head, mRxBuffer_Tail;

		size_t mPredictedSize;
		
		// number of milliseconds to wait for the next block before aborting
		// timed read
		uint32_t mTimeout;
		// used for timeout measurement
		uint32_t mStartMillis;
		// private method to read stream with timeout
		size_t timedRead(uint8_t *dst = NULL);
		// private method to peek stream with timeout
		size_t timedPeek(uint8_t *&dst);
		
    void setError(ErrorState error) { mErrorState = error; }

		// you need to tell us when the block is end,
		// you can use terminator character or length field in 'blockPart'.
		// if you don't know, please return 0.
		// if you want to continue to receive, you can pass 'size + 1'.
		// if you want to stop receiving, give us 'size' or a number
		// less than 'size', but not 0.
		// 'size' is size of the block part we've read.
		virtual size_t predictSize(uint8_t *blockPart, size_t size) = 0;
		
		// if you have any function to check the block is right, please
		// override it.
		virtual bool checkSum(uint8_t *block, size_t size);

	public:
		// attach BlockStream to a Stream. we work on a byte-based stream.
		// we need a header, the start of the block, use a string.
		// we get to know how large of one block by 'blockSize', includes
		// header and tail.
		// 'bufferSize' is maximum blocks we can hold.
		BlockStream(Stream &bus, const uint8_t *header, size_t blockSize,
		  uint8_t bufferSize = DEFAULT_BUFFER_SIZE, uint32_t timeout = 1000);

		ErrorState getError() const { return mErrorState; }
    void clearError() { mErrorState = NO_ERRORS; }
		
		// call it when the stream receive something.
		// if the 'readBytesLimit' is 0, it will read all available bytes.
		void listener(size_t readBytesLimit = 0);
		
		uint8_t available();
		size_t read(uint8_t *dst = NULL);
		size_t peek(uint8_t *&dst);
		size_t write(const uint8_t *block, size_t size);
		void flush();
		
		// if the block is complete receive, it returns False.
		bool isReading();
		
		virtual ~BlockStream();
};

#endif /* BLOCKSTREAM_H_ */
