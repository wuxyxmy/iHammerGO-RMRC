/*******************************************************************************
 * File name: Thread.cpp
 * Header file: Thread.cpp
 * Created on: 2018年5月5日
 * Author: Wuxy
 * 简单“线程”库
 *
 * History:
 *  v4
 *    2018-05-29 添加线程ID及其getter方法，删除SchedulerClass::remove(Thread*)方法
 ******************************************************************************/
#include "Thread.h"

/*******************Debugging**************************************************/
//#define DEBUG

// Macros
#ifdef DEBUG
#include <utility/SdFatUtil.h>
#define debugPool()                                               \
do {                                                              \
  Serial.println();                                               \
  ListNode __cur = mPool;                                         \
  for (size_t i = 0; __cur != NULL; __cur = __cur->next, i ++) {  \
    Serial.print(i);                                              \
    Serial.print(F("\t0x"));                                      \
    Serial.print((uint16_t)__cur, HEX);                           \
    Serial.print(F("\t0x"));                                      \
    Serial.print((uint16_t)__cur->thread, HEX);                   \
    Serial.print('\t');                                           \
    if(__cur->thread) Serial.print(__cur->thread->getId(), DEC);  \
    if(__cur == mCurrent) Serial.print(F("\t<-Cur"));             \
    Serial.println();                                             \
  }                                                               \
  Serial.print(mRunningThreads, DEC);                             \
  Serial.print(F(" Running\tFree "));                              \
  Serial.print(FreeRam(), DEC);                                   \
  Serial.println();                                               \
} while (0)
#else /* DEBUG */
#define debugPool()
#endif /* DEBUG */

/*******************Scheduler Entity*******************************************/
static SchedulerClass Scheduler;

/*******************Thread*****************************************************/
//--------Static Variables----------------------------------------------------//
uint16_t Thread::threadSeqNumber = 0;

//--------Constructors--------------------------------------------------------//
Thread::Thread(void (*target)(), uint32_t millisInterval) :
  mTid(++threadSeqNumber),
  mStarted(false),
  mAlive(false),
  mDestoryed(false),
  mTarget(target),
  mMillisInterval(millisInterval),
  mNextMillis(0)
{
}

//--------Getter & Setter-----------------------------------------------------//
uint16_t Thread::getId() {
  return mTid;
}

void Thread::setInterval(uint32_t millisInterval) {
  mMillisInterval = millisInterval;
}

uint32_t Thread::getInterval() {
  return mMillisInterval;
}

inline bool Thread::shouldRun() {
  if (!shouldRun(millis())) return false;
  mNextMillis = millis() + mMillisInterval;
  return true;
}

bool Thread::shouldRun(uint32_t millis) {
  return mAlive && millis >= mNextMillis;
}

bool Thread::isAlive() {
  return mAlive;
}

bool Thread::isDestoryed() {
  return mDestoryed;
}

//--------Member Functions----------------------------------------------------//
void Thread::start() {
  if (mDestoryed) return;
  mAlive = true;
  if (mStarted) return;
  Scheduler.add(this);
  mStarted = true;
}

void Thread::run() {
  if(mTarget != NULL) mTarget();
}

void Thread::stop() {
  mAlive = false;
}

Thread::~Thread() {
  mAlive = false;
  mDestoryed = true;
}

/*******************Scheduler**************************************************/
//--------Constructors--------------------------------------------------------//
inline SchedulerClass::SchedulerClass() {
  mRunningThreads = 0;
  mPool = new ListNodeClass();
  mPool->thread = NULL;
  mPool->next = NULL;
  mCurrent = mPool;
}

//--------Member Functions----------------------------------------------------//
inline void SchedulerClass::add(Thread *thread) {
  ListNode n = new ListNodeClass();
  n->thread = thread;
  n->next = mPool->next;
  mPool->next = n;
  mRunningThreads ++;

  debugPool();
}

inline void SchedulerClass::run() {
  if (mRunningThreads == 0) return;

  // we needn't check if 'mCurrent->next->thread' is NULL. only header
  // node's field 'thread' is NULL.
  if (mCurrent->next != NULL && mCurrent->next->thread->isDestoryed()) {
    // remove the node.
    ListNode node = mCurrent->next;
    mCurrent->next = mCurrent->next->next;
    delete node;
    mRunningThreads --;

    debugPool();
  }
  // run each thread in the pool.
  mCurrent = mCurrent->next;
  // reach the end, restore the pointer.
  if (mCurrent == NULL) mCurrent = mPool;
  // can't access header node's field 'thread', witch is NULL.
  else if (mCurrent->thread->shouldRun()) mCurrent->thread->run();
}

/*******************Override Loop**********************************************/
void loop() {
  Scheduler.run();
}

/******************************************************************************/
