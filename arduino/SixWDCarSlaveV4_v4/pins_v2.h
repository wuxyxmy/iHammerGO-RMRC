/*******************************************************************************
 * File name: pins_v2.h
 * Created on: 2017/10/15
 * Author: Wuxy
 * 扩展板(PCB, V1.0)接口定义
 *
 * History:
 *  v20
 *    2018/05/29 文件重构，全部使用常量替代宏
 *    2018/05/28 新的扩展板接口定义(PCB, V1.0)
 ******************************************************************************/
#ifndef PINS_V2_H_
#define PINS_V2_H_

#include <Arduino.h>

static HardwareSerial &Host     = Serial2;
static HardwareSerial &ServoBus = Serial3;
static HardwareSerial &Debug    = Serial;

//--------PS/2 Ports----------------------------------------------------------//
static const uint8_t PS2_DAT = 37;
static const uint8_t PS2_CMD = 35;
static const uint8_t PS2_CS  = 33;
static const uint8_t PS2_CLK = 31;

//--------Motor Ports---------------------------------------------------------//
// TB6612FNG #1
static const uint8_t MOTOR_1_PWMA = 9;
static const uint8_t MOTOR_1_PWMB = 8;
static const uint8_t MOTOR_1_A1   = A1;
static const uint8_t MOTOR_1_A2   = A2;
static const uint8_t MOTOR_1_B2   = A3;
static const uint8_t MOTOR_1_B1   = A4;

// TB6612FNG #2
static const uint8_t MOTOR_2_PWMA = 5;
static const uint8_t MOTOR_2_PWMB = 4;
static const uint8_t MOTOR_2_A1   = A5;
static const uint8_t MOTOR_2_A2   = A6;
static const uint8_t MOTOR_2_B2   = A7;
static const uint8_t MOTOR_2_B1   = A8;

// TB6612FNG #3
static const uint8_t MOTOR_3_PWMA = 7;
static const uint8_t MOTOR_3_PWMB = 6;
static const uint8_t MOTOR_3_A1   = 36;
static const uint8_t MOTOR_3_A2   = 34;
static const uint8_t MOTOR_3_B2   = 32;
static const uint8_t MOTOR_3_B1   = 30;

// TB6612FNG #4
static const uint8_t MOTOR_4_PWMA = 3;
static const uint8_t MOTOR_4_PWMB = 2;
static const uint8_t MOTOR_4_A1   = 22;
static const uint8_t MOTOR_4_A2   = 24;
static const uint8_t MOTOR_4_B2   = 26;
static const uint8_t MOTOR_4_B1   = 28;

//--------Servo Ports---------------------------------------------------------//
static const uint8_t SERVO_EX1_PIN = 13;
static const uint8_t SERVO_EX2_PIN = 12;
static const uint8_t SERVO_EX3_PIN = 11;
static const uint8_t SERVO_EX4_PIN = 10;

#endif /* PINS_V2_H_ */
