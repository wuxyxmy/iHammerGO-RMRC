/*******************************************************************************
 * File name: BlockStream.cpp
 * Header file: BlockStream.h
 * Created on: 2018年2月2日
 * Author: Wuxy
 * 数据块流
*******************************************************************************/
#include "BlockStream.h"

/*******************Class Protected********************************************/
size_t BlockStream::timedRead(uint8_t *dst) {
  size_t size = 0;
  mStartMillis = millis();
  do {
    size = read(dst);
    if (size > 0) return size;

    // you can use listener() in yield() to run like a multi-thread system.
    if (serialEventRun) serialEventRun();
    yield();
  } while(millis() - mStartMillis < mTimeout);

  // timeout.
  return 0;
}

size_t BlockStream::timedPeek(uint8_t* &dst) {
  size_t size = 0;
  mStartMillis = millis();
  do {
    size = peek(dst);
    if (size > 0) return size;

    // you can use listener() in yield() to run like a multi-thread system.
    if (serialEventRun) serialEventRun();
    yield();
  } while(millis() - mStartMillis < mTimeout);

  // timeout.
  return 0;
}

/*******************Class Public***********************************************/
//--------Constructors--------------------------------------------------------//
BlockStream::BlockStream(
	Stream &bus, const uint8_t *headers, size_t blockSize,
	uint8_t bufferSize, uint32_t timeout) : 
  mErrorState(NO_ERRORS),
	mBus(bus),
	mBlockSize(blockSize),
	mHeaders(headers),
  mBufferSize(bufferSize + 1), // we use a ring buffer.
	mRxBuffer(new BlockNode[mBufferSize]),
	mRxBuffer_Head(0), mRxBuffer_Tail(0),
	mPredictedSize(0),
	mTimeout(timeout), mStartMillis(0)
{
	// we'd better not to use memset.
	for (uint8_t i = 0; i < mBufferSize; i ++) {
		mRxBuffer[i].block = NULL;
		mRxBuffer[i].size = 0;
	}
}

//--------Implementations-----------------------------------------------------//
bool BlockStream::checkSum(uint8_t */*block*/, size_t /*size*/) {
	return true;
}

//--------Common Function-----------------------------------------------------//
void BlockStream::listener(size_t readBytesLimit) {
	while (mBus.available()) {
		// rx buffer is full, we'll wait.
		if ((mRxBuffer_Head + 1) % mBufferSize == mRxBuffer_Tail) {
			mErrorState = ERROR_RX_BUFFER_FULL;
			break;
		}
		
		// useful to prevent receiving errors.
		delayMicroseconds(10);
		int16_t c = mBus.peek();
		
		// help variable.
		BlockNode &cur = mRxBuffer[mRxBuffer_Head];
		
		/* warning: "Suggested parenthesis around expression 'curBlock.block == NULL
		 *           && mHeader == NULL && curBlock.size == 0'"
		 * with old condition:
		 * curBlock.block == NULL && mHeader != NULL && c == mHeader[0] ||
     * curBlock.block == NULL && mHeader == NULL && curBlock.size == 0
     *
     * --> A B C + A !B D
     * = A (B C + !B D)
     * [
     *   B C D  BC+!BD
		 *   0 0 0  0
		 *   0 0 1  1
		 *   0 1 0  0
		 *   0 1 1  1
		 *   1 0 0  0
		 *   1 0 1  0
		 *   1 1 0  1
		 *   1 1 1  1
		 * ]
		 * = A D !(B + !C) -->
		 *
		 * new condition:
		 * curBlock.block == NULL && curBlock.size == 0 && !(mHeader != NULL && c != mHeader[0])
		 */

    // when we receive 1st header, get memory for it.
    // (mHeader can be NULL, if it is, we'll also get memory)
		if (cur.size == 0 && !(mHeaders != NULL && c != mHeaders[0])) {
			cur.size = 0;
			cur.block = new uint8_t[mBlockSize];
			memset(cur.block, 0, mBlockSize * sizeof(uint8_t));
			mPredictedSize = mBlockSize;
		}
		// if it isn't nth header when we receive nth byte, drop it.
		else if (mHeaders != NULL && cur.size < strlen((char*)mHeaders)) {
			if (c != mHeaders[cur.size]) {
				cur.size = 0;
				delete[] cur.block;
				cur.block = NULL;
			}
		}
		
		// store the byte.
		if (cur.block != NULL) {
			cur.block[cur.size ++] = c;
			
			// predict block size.
			size_t l = predictSize(cur.block, cur.size);
			if (l > 0 && l <= mBlockSize) mPredictedSize = l;
		}
		
		// if we reach the end of block, stop receiving immediately.
		if (cur.size >= mPredictedSize) {
			if (checkSum(cur.block, cur.size)) {
				mRxBuffer_Head = (mRxBuffer_Head + 1) % mBufferSize;
			} else {
				mErrorState = ERROR_BAD_CHECKING_SUM;

				cur.size = 0;
				delete[] cur.block;
				cur.block = NULL;
			}
		}
		
		mBus.read();
		
		if (readBytesLimit != 0 && --readBytesLimit == 0) break;
	}
}

uint8_t BlockStream::available() {
	return ((uint8_t)(mBufferSize + mRxBuffer_Head - mRxBuffer_Tail)) % mBufferSize;
}

size_t BlockStream::read(uint8_t *dst) {
	if (mRxBuffer_Tail == mRxBuffer_Head) {
		// we don't have any blocks, or the block is still receiving.
		return 0;
	} else {
		// help variable.
		BlockNode &cur = mRxBuffer[mRxBuffer_Tail];
		
		size_t size = 0;
		// avoid to null pointer exception, and allows to peek()/read().
		if(dst != NULL) {
			memcpy(dst, cur.block, cur.size);
			size = cur.size;
		}
		
		// free memory.
		cur.size = 0;
		delete[] cur.block;
		cur.block = NULL;
		
		mRxBuffer_Tail = (mRxBuffer_Tail + 1) % mBufferSize;
		
		return size;
	}
}

size_t BlockStream::peek(uint8_t *&dst) {
	if (mRxBuffer_Tail == mRxBuffer_Head) {
		return 0;
	} else {
		dst = mRxBuffer[mRxBuffer_Tail].block;
		return mRxBuffer[mRxBuffer_Tail].size;
	}
}

size_t BlockStream::write(const uint8_t *block, size_t size) {
	return mBus.write(block, size);
}

void BlockStream::flush() {
	mBus.flush();
}

bool BlockStream::isReading() {
	return mRxBuffer[mRxBuffer_Head].size > 0;
}

//--------Destructor----------------------------------------------------------//
BlockStream::~BlockStream() {
  while (mRxBuffer_Tail != mRxBuffer_Head) {
    BlockNode &cur = mRxBuffer[mRxBuffer_Tail];

    // free memory.
    cur.size = 0;
    delete[] cur.block;
    cur.block = NULL;

    mRxBuffer_Tail = (mRxBuffer_Tail + 1) % mBufferSize;
  }

  delete[] mRxBuffer;
}

/******************************************************************************/
