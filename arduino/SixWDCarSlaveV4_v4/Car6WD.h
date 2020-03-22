/*******************************************************************************
 * File name: Car6WD.h
 * Source file: Car6WD.cpp
 * Created on: 2017年10月15日
 * Author: Wuxy
 * 6WD小车电机驱动库 for 扩展板V3
 *
 * History:
 *  v6
 *    2018-05-29 修改枚举类型命名空间(Car6WD::CarMotorSwhitcher)
 *******************************************************************************/
#ifndef CAR6WD_H_
#define CAR6WD_H_

#include "DCMotor.h"

#include "pins.h"

class Car6WD {
  public:
    enum CarMotorSwhitcher { L1 = 0, R1, L2, R2, L3, R3, NUM_OF_MOTORS};

  protected:
    DCMotor mMotors[NUM_OF_MOTORS];

  public:
    void begin();

    void run(CarMotorSwhitcher which, int16_t speed);
    void run(int16_t speedL1, int16_t speedR1, int16_t speedL2, int16_t speedR2, int16_t speedL3, int16_t speedR3);
    void run(int16_t speedL, int16_t speedR);

    void stop(CarMotorSwhitcher which, uint8_t hard);
    void stop(uint8_t hardL1, uint8_t hardR1, uint8_t hardL2, uint8_t hardR2, uint8_t hardL3, uint8_t hardR3);
    void stop(uint8_t hardL, uint8_t hardR);
};

#endif /* CAR6WD_H_ */
