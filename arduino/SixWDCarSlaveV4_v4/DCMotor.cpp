/*******************************************************************************
 * File name: DCMotor.cpp
 * Header file: DCMotor.h
 * Created on: 2017/10/15
 * Author: Wuxy
 * 直流电机库
 *
 * History:
 *  v6
 *    2018/06/13 [header file]在stop(uint8_t)中添加hard参数缺省值
 *    2018/05/29 增加辅助常量(MASK_A, MASK_B)
 *******************************************************************************/
#include "DCMotor.h"

// For faster motor control.
static const uint8_t FORWARD  = 0b10;
static const uint8_t RELEASE  = 0b11;
static const uint8_t STOP     = 0b11;
static const uint8_t BACKWARD = 0b01;

static const uint8_t MASK_A = 0b10;
static const uint8_t MASK_B = 0b01;

DCMotor::DCMotor(uint8_t pinA, uint8_t pinB, uint8_t pinPWM) {
  attach(pinA, pinB, pinPWM);
}

void DCMotor::attach(uint8_t pinA, uint8_t pinB, uint8_t pinPWM) {
  mPinA = pinA;
  mPinB = pinB;
  mPinPWM = pinPWM;

  pinMode(mPinA, OUTPUT);
  pinMode(mPinB, OUTPUT);
  pinMode(mPinPWM, OUTPUT);
}

inline void DCMotor::run(uint8_t dir, uint8_t speed) {
  digitalWrite(mPinA, dir & MASK_A);
  digitalWrite(mPinB, dir & MASK_B);
  analogWrite(mPinPWM, speed);
}

void DCMotor::run(int16_t speed) {
  speed = constrain(speed, -255, 255);

  if (speed == 1 || speed == -1) run(RELEASE, 255);
  else if (speed > 0) run(FORWARD, speed);
  else if (speed < 0) run(BACKWARD, -speed);
  else run(STOP, 255);
}

void DCMotor::stop(uint8_t /*hard*/) {
  run(STOP, 255); // for TB6612FNG, break has no hard (break with maximum hard).
}

inline void DCMotor::release() {
  run(RELEASE, 0); // for TB6612FNG, break has no hard (break with maximum hard).
}

