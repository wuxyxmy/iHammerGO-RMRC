/*******************************************************************************
 * File name: DCMotor.h
 * Source file: DCMotor.cpp
 * Created on: 2017/10/15
 * Author: Wuxy
 * 直流电机库
 *
 * History:
 *  v6
 *    2018/06/13 在stop(uint8_t)中添加hard参数缺省值
 *    2018/05/29 [source file]增加辅助常量
 *******************************************************************************/
#ifndef DCMOTOR_H_
#define DCMOTOR_H_

#include <Arduino.h>

class DCMotor {
  protected:
    uint8_t mPinA, mPinB, mPinPWM;

    void run(uint8_t dir, uint8_t speed);

  public:
    DCMotor(uint8_t pinA = NOT_A_PIN, uint8_t pinB = NOT_A_PIN, uint8_t pinPWM = NOT_A_PIN);

    void attach(uint8_t pinA, uint8_t pinB, uint8_t pinPWM);
    void run(int16_t speed);
    void stop(uint8_t hard = 255);
    void release();
};

#endif /* DCMOTOR_H_ */
