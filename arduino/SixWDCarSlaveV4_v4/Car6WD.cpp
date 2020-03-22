/*******************************************************************************
 * File name: Car6WD.cpp
 * Header file: Car6WD.h
 * Created on: 2017年10月15日
 * Author: Wuxy
 * 6WD小车电机驱动库 for 扩展板V3
 *
 * History:
 *  v6
 *    2018-05-29 [header file]修改枚举类型访问控制
*******************************************************************************/
#include "Car6WD.h"

static const uint8_t MOTOR[Car6WD::NUM_OF_MOTORS][3] = {
  {MOTOR_3_A1, MOTOR_3_A2, MOTOR_3_PWMA},
  {MOTOR_3_B1, MOTOR_3_B2, MOTOR_3_PWMB},
  {MOTOR_4_A1, MOTOR_4_A2, MOTOR_4_PWMA},
  {MOTOR_4_B1, MOTOR_4_B2, MOTOR_4_PWMB},
  {MOTOR_1_A1, MOTOR_1_A2, MOTOR_1_PWMA},
  {MOTOR_1_B1, MOTOR_1_B2, MOTOR_1_PWMB},
};

void Car6WD::begin() {
	for(uint8_t i = 0; i < NUM_OF_MOTORS; i ++) {
		mMotors[i].attach(MOTOR[i][0], MOTOR[i][1], MOTOR[i][2]);
	}
}

void Car6WD::run(CarMotorSwhitcher which, int16_t speed) {
	mMotors[which].run(speed);
}

void Car6WD::run(int16_t speedL1, int16_t speedR1, int16_t speedL2, int16_t speedR2, int16_t speedL3, int16_t speedR3) {
	run(L1, speedL1);
	run(R1, speedR1);
	run(L2, speedL2);
	run(R2, speedR2);
	run(L3, speedL3);
	run(R3, speedR3);
}

void Car6WD::run(int16_t speedL, int16_t speedR) {
	run(speedL, speedR, speedL, speedR, speedL, speedR);
}

void Car6WD::stop(CarMotorSwhitcher which, uint8_t hard) {
	mMotors[which].stop(hard);
}

void Car6WD::stop(uint8_t hardL1, uint8_t hardR1, uint8_t hardL2, uint8_t hardR2, uint8_t hardL3, uint8_t hardR3) {
  if (hardL1) stop(L1, hardL1);
  if (hardR1) stop(R1, hardR1);
  if (hardL2) stop(L2, hardL2);
  if (hardR2) stop(R2, hardR2);
  if (hardL3) stop(L3, hardL3);
  if (hardR3) stop(R3, hardR3);
}

void Car6WD::stop(uint8_t hardL, uint8_t hardR) {
	stop(hardL, hardR, hardL, hardR, hardL, hardR);
}
