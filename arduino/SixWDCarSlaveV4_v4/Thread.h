/*******************************************************************************
 * File name: Thread.h
 * Source file: Thread.cpp
 * Created on: 2018年5月5日
 * Author: Wuxy
 * 简单“线程”库
 *
 * History:
 *  v4
 *    2018-05-29 添加线程ID及其getter方法，删除SchedulerClass::remove(Thread*)方法
 ******************************************************************************/
#ifndef THREAD_H_
#define THREAD_H_

#include <Arduino.h>

class Thread {
  friend class SchedulerClass;

  private:
    // For generating thread ID
    static uint16_t threadSeqNumber;
    // Thread ID
    uint16_t mTid;

  protected:
    bool mStarted;
    bool mAlive;
    bool mDestoryed;
    void (*mTarget)();
    uint32_t mMillisInterval;
    uint32_t mNextMillis;

  public:
    Thread(void (*target)(), uint32_t millisInterval = 0);

    uint16_t getId();
    void setInterval(uint32_t millisInterval);
    uint32_t getInterval();
    bool shouldRun();
    bool shouldRun(uint32_t millis);

    bool isAlive();
    bool isDestoryed();

    void start();
    void run();
    void stop();
    void destory();

    ~Thread();
};

/*******************Private Class**********************************************/
class SchedulerClass {
  friend class Thread;

  protected:
    typedef struct ListNodeClass {
        Thread *thread;
        struct ListNodeClass *next;
    } *ListNode;

    ListNode mPool;
    size_t mRunningThreads;
    ListNode mCurrent;

  public:
    SchedulerClass();

    void add(Thread *thread);
    void run();
};

#endif /* THREAD_H_ */
