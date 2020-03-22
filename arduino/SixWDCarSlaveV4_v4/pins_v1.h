/*******************************************************************************
 * File name: pins_v1.h
 * Created on: 2018/06/02
 * Author: Wuxy
 * 扩展板(old v3)接口定义
 *
 * History:
 *  v1
 *    2018/06/02 从pins_v2.h创建文件
 ******************************************************************************/
#ifndef PINS_V1_H_
#define PINS_V1_H_

#include <Arduino.h>

static HardwareSerial &Host     = Serial2;
static HardwareSerial &ServoBus = Serial3;
static HardwareSerial &Debug    = Serial;

//--------PS/2 Ports----------------------------------------------------------//
static const uint8_t PS2_DAT = A1;
static const uint8_t PS2_CMD = A2;
static const uint8_t PS2_CS  = A3;
static const uint8_t PS2_CLK = A4;

//--------Motor Ports---------------------------------------------------------//
// TB6612FNG #1
static const uint8_t MOTOR_1_PWMA = 2;
static const uint8_t MOTOR_1_PWMB = 3;
static const uint8_t MOTOR_1_A2   = 22;
static const uint8_t MOTOR_1_A1   = 24;
static const uint8_t MOTOR_1_B1   = 26;
static const uint8_t MOTOR_1_B2   = 28;

// TB6612FNG #2
static const uint8_t MOTOR_2_PWMA = 4;
static const uint8_t MOTOR_2_PWMB = 5;
static const uint8_t MOTOR_2_A2   = 30;
static const uint8_t MOTOR_2_A1   = 32;
static const uint8_t MOTOR_2_B1   = 34;
static const uint8_t MOTOR_2_B2   = 36;

// TB6612FNG #3
static const uint8_t MOTOR_3_PWMA = 6;
static const uint8_t MOTOR_3_PWMB = 7;
static const uint8_t MOTOR_3_A2   = 23;
static const uint8_t MOTOR_3_A1   = 25;
static const uint8_t MOTOR_3_B1   = 27;
static const uint8_t MOTOR_3_B2   = 29;

// TB6612FNG #4
static const uint8_t MOTOR_4_PWMA = 8;
static const uint8_t MOTOR_4_PWMB = 9;
static const uint8_t MOTOR_4_A2   = 31;
static const uint8_t MOTOR_4_A1   = 33;
static const uint8_t MOTOR_4_B1   = 35;
static const uint8_t MOTOR_4_B2   = 37;

//--------Servo Ports---------------------------------------------------------//
static const uint8_t SERVO_EX1_PIN = NOT_A_PIN;
static const uint8_t SERVO_EX2_PIN = NOT_A_PIN;
static const uint8_t SERVO_EX3_PIN = NOT_A_PIN;
static const uint8_t SERVO_EX4_PIN = NOT_A_PIN;

#endif /* PINS_V1_H_ */
