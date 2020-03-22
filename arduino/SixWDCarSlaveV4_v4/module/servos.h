/*******************************************************************************
 * File name: servos.h
 * Created on: 2018/06/14 01:02:23
 * Author: Wuxy
 * 舵机控制模块
 * 
 * History:
 * 	v1
 * 		2018/06/13 分离原主程序中的舵机控制代码
*******************************************************************************/
#ifndef SERVOS_H_
#define SERVOS_H_

#include "../SerialServo.h"

void servos_begin();
void servos_reset();
void servos_write(uint8_t id, uint16_t position, uint16_t time);
void servos_run(uint8_t id, int8_t speed);

int16_t servos_getTemperature(uint8_t id);
int16_t servos_getVoltage(uint8_t id);
int16_t servos_getGoalPos(uint8_t id);
int16_t servos_getCurrentPos(uint8_t id);

extern const uint8_t NUM_OF_SERIAL_SERVOS;
extern const uint8_t NUM_OF_COMMON_SERVOS;
extern const uint8_t NUM_OF_LOCABLE_SERVOS;
extern const uint8_t NUM_OF_MOTOR_SERVOS;
extern const uint8_t NUM_OF_ALL_SERVOS;
extern const uint8_t NUM_OF_SERVO_SPEEDS;
extern SerialServo serialServos[];
extern uint16_t servoPositions[];

#endif /* SERVOS_H_ */
