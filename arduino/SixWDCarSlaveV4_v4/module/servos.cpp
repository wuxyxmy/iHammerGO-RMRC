/*******************************************************************************
 * File name: servos.cpp
 * Created on: 2018/06/13 22:55:52
 * Author: Wuxy
 * 舵机控制模块
 * 
 * History:
 * 	v1
 * 		2018/06/13 分离原主程序中的舵机控制代码
*******************************************************************************/
#include "servos.h"

#include <Arduino.h>

// Libraries
#include <Servo.h>

// Project
#include "../pins.h"
#include "../SerialServoBus.h"
#include "../SerialServo.h"
#include "../DCMotor.h"
#include "../Thread.h"
#include "../utility/Range.h"

/*******************Constants**************************************************/
/* .--------------------------------------------------------------------------.
 * | Servo ID   Type             Port          Use                            |
 * | --------   --------------   -----------   ------------------------------ |
 * |    1       Serial Servo     Bus (ID=1)    Robot arm joint #1             |
 * |    2       Serial Servo     Bus (ID=2)    Robot arm joint #2             |
 * |    3       Serial Servo     Bus (ID=3)    Robot arm joint #3             |
 * |    4       Serial Servo     Bus (ID=4)    Robot arm joint #4             |
 * |    5       Servo            Servo 1       Camera platform                |
 * |    6       Servo as Motor   Servo 2       Manipulator                    |
 * |    7       Motor            #2 TB6612 A   Rotating platform of robot arm |
 * |    8       Motor            #2 TB6612 B   Winch                          |
 * '--------------------------------------------------------------------------'
 */
const uint8_t NUM_OF_SERIAL_SERVOS  = 4;
const uint8_t NUM_OF_COMMON_SERVOS  = 2;
const uint8_t NUM_OF_LOCABLE_SERVOS = 5; // one of the 'common servos' is 'servo as motor', witch is unlocable.
const uint8_t NUM_OF_MOTOR_SERVOS   = 3;
const uint8_t NUM_OF_ALL_SERVOS     = 8;

//--------All Servos----------------------------------------------------------//
const bool SERVO_DIR_REVERSE[NUM_OF_ALL_SERVOS] = { 0, 1, 0, 1, 0, 0, 0, 0 };
// Speed
const uint8_t NUM_OF_SERVO_SPEEDS = 5;
const int8_t  SERVO_SPEED_MAP[NUM_OF_SERVO_SPEEDS]       = { 1,  3,  5,   8,   12  };
const int16_t SERVO_MOTOR_SPEED_MAP[NUM_OF_SERVO_SPEEDS] = { 30, 50, 100, 130, 180 };

//--------Locable Servos------------------------------------------------------//
const uint16_t SERVO_POSITION_DEFAULT[NUM_OF_LOCABLE_SERVOS] = { 200,  930, 950, 880,  500  };
const uint16_t SERVO_POSITION_MAX[NUM_OF_LOCABLE_SERVOS]     = { 1000, 930, 950, 1000, 1000 };
const uint16_t SERVO_POSITION_MIN[NUM_OF_LOCABLE_SERVOS]     = { 200,  0,   50,  0,    0    };

const uint16_t SERVO_RUNNING_INTERVAL = 50; // ms

//--------Serial Servos-------------------------------------------------------//
// Basic
const uint8_t  SERVO_ID[NUM_OF_SERIAL_SERVOS] = { 1, 2, 3, 4 };
// Setup
const uint32_t SERVO_MILLIS_SETUP = 2000; // ms
// Communication
const uint32_t SERVO_FRAME_TIMEOUT      = 30; // ms
const uint32_t SERVO_BUS_CLEAN_INTERVAL = 3000; // ms
// State & Warning
const uint32_t SERVO_DAEMON_INTERVAL   = 2000; // ms
const uint8_t  SERVO_DAEMON_TEMP_LIMIT = 65; // 'C

/*******************Variables**************************************************/
SerialServoBus sBus(ServoBus, SERVO_FRAME_TIMEOUT);
SerialServo    serialServos[NUM_OF_SERIAL_SERVOS];
Servo          servos      [NUM_OF_COMMON_SERVOS]; // 摄像头[0], 爪子[1]
DCMotor        motorServos [NUM_OF_MOTOR_SERVOS];  // 转台[0], 绞盘[1]

//--------Locable Servos------------------------------------------------------//
uint16_t servoPositions[NUM_OF_LOCABLE_SERVOS] = { 500, 500, 500, 500, 500 };
int8_t   servoSpeeds   [NUM_OF_LOCABLE_SERVOS] = { 0 };

static void servoThreadRun();
Thread servoThread(servoThreadRun, SERVO_RUNNING_INTERVAL);
static void servoDaemonThreadRun();
Thread servoDaemonThread(servoDaemonThreadRun, SERVO_DAEMON_INTERVAL);
static void busCleaningThreadRun();
Thread busCleaningThread(busCleaningThreadRun, SERVO_BUS_CLEAN_INTERVAL);

/*******************Functions**************************************************/
void servos_begin() {
  ServoBus.begin(115200);

  serialServos[0].attach(sBus, SERVO_ID[0]);
  serialServos[1].attach(sBus, SERVO_ID[1]);
  serialServos[2].attach(sBus, SERVO_ID[2]);
  serialServos[3].attach(sBus, SERVO_ID[3]);
  servos[0].attach(SERVO_EX1_PIN);
  servos[1].attach(SERVO_EX2_PIN);
  motorServos[0].attach(MOTOR_2_A1, MOTOR_2_A2, MOTOR_2_PWMA);
  motorServos[1].attach(MOTOR_2_B1, MOTOR_2_B2, MOTOR_2_PWMB);

  servoDaemonThread.start();
  busCleaningThread.start();
}

void servos_reset(){
  // reset all servos with a slow speed.
  memcpy(servoPositions, SERVO_POSITION_DEFAULT, sizeof(SERVO_POSITION_DEFAULT));
  for (uint8_t i = 0; i < NUM_OF_LOCABLE_SERVOS; i ++) {
    servos_write(i + 1, servoPositions[i], SERVO_MILLIS_SETUP);
  }
  servos_run(6, 0);
  servos_run(7, 0);
  servos_run(8, 0);
}

void servos_write(uint8_t id, uint16_t position, uint16_t time) {
  if (In_Range(id, 1, NUM_OF_SERIAL_SERVOS))
    serialServos[id - 1].writeRaw(position, time);
  else if (id == 5)
    servos[5 - NUM_OF_LOCABLE_SERVOS].write(position * 2 + 500);
}

void servos_run(uint8_t id, int8_t speed) {
  if (SERVO_DIR_REVERSE[id - 1]) speed = -speed;

  // serial servos and common servos  (#1 - #5)
  if (In_Range(id, 1, NUM_OF_LOCABLE_SERVOS)) {
    if (speed > 0)  servoSpeeds[id - 1] = SERVO_SPEED_MAP[speed];
    if (speed == 0) servoSpeeds[id - 1] = 0;
    if (speed < 0)  servoSpeeds[id - 1] = -SERVO_SPEED_MAP[-speed];
    if (speed != 0) servoThread.start();
  }
  // servo #6 is belong to 'Servo as motor'
  else if (id == 6) {
    if (speed > 0)  servos[6 - NUM_OF_LOCABLE_SERVOS].write(180);
    if (speed == 0) servos[6 - NUM_OF_LOCABLE_SERVOS].write(90);
    if (speed < 0)  servos[6 - NUM_OF_LOCABLE_SERVOS].write(0);
  }
  // motor 'servos' (#7 - #8)
  else if (id <= NUM_OF_ALL_SERVOS) {
    if (speed > 0)  motorServos[id - 7].run(SERVO_MOTOR_SPEED_MAP[speed]);
    if (speed == 0) motorServos[id - 7].run(0);
    if (speed < 0)  motorServos[id - 7].run(-SERVO_MOTOR_SPEED_MAP[-speed]);
  }
}


int16_t servos_getTemperature(uint8_t id) {
  return In_Range(id, 1, NUM_OF_SERIAL_SERVOS) ?
      serialServos[id - 1].readTemperature() : SerialServo::FAIL_VALUE;
}

int16_t servos_getVoltage(uint8_t id) {
  return In_Range(id, 1, NUM_OF_SERIAL_SERVOS) ?
      serialServos[id - 1].readVoltage() : SerialServo::FAIL_VALUE;
}

int16_t servos_getGoalPos(uint8_t id) {
  return In_Range(id, 1, NUM_OF_LOCABLE_SERVOS) ?
      servoPositions[id - 1] : SerialServo::FAIL_VALUE;
}

int16_t servos_getCurrentPos(uint8_t id) {
  return In_Range(id, 1, NUM_OF_SERIAL_SERVOS) ?
      serialServos[id - 1].read() : SerialServo::FAIL_VALUE;
}

//--------Handlers (Events)---------------------------------------------------//
// ServoBus Listener
void serialEvent3() {
  sBus.listener();
}

//--------Threads-------------------------------------------------------------//
static void servoThreadRun() {
  // we might haven't any servos to run.
  bool shouldRunServos = false;

  for (uint8_t i = 0; i < NUM_OF_LOCABLE_SERVOS; i++) {
    servoPositions[i] += servoSpeeds[i];
    // the condition also can be true when the speed is minus.
    // because at that time (servoAngles[i] = 0) 'servoAngles[i]' will
    // be a overflow value, we ignore the sign and its value must be
    // around 0xFFFF. of course, it is much greater than 1000.
    if (!In_Range(servoPositions[i], SERVO_POSITION_MIN[i], SERVO_POSITION_MAX[i])) {
      // reach the servo angle limit.
      // restore the last value, in order to avoid problems.
      servoPositions[i] -= servoSpeeds[i];
      // stop the servo.
      servoSpeeds[i] = 0;
    }

    // if we still have some servos to run ?
    if (servoSpeeds[i] != 0) shouldRunServos = true;

    servos_write(i + 1, servoPositions[i], SERVO_RUNNING_INTERVAL);
  }

  // no servo to run, stop the thread to save CPU time.
  if (!shouldRunServos) servoThread.stop();
}

static void servoDaemonThreadRun() {
  for (uint8_t i = 0; i < NUM_OF_SERIAL_SERVOS; i++) {
    int16_t temp = serialServos[i].readTemperature();
    // this servo must be blocked, so turn it off to protect the servo.
    if (temp != SerialServo::FAIL_VALUE && temp >= SERVO_DAEMON_TEMP_LIMIT)
      serialServos[i].powerOff();
  }
}

static void busCleaningThreadRun() {
  // sometimes there is garbage in the buffer, it will fill up the buffer
  // and we could not get any packet from the buffer.
  sBus.clear();
}
