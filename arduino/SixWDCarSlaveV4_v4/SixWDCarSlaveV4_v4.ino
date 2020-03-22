/*******************************************************************************
 * File name: SixWDCarSlaveV4_v4.ino
 * Created on: 2018/06/19
 * Board: Arduino Mega 2560
 * Author: Wuxy
 *
 * 6WD小车下位机 for 电机扩展板V3
 * History:
 *  SixWDCarSlaveV4
 *  v4
 *    2018/06/19 存档
 *  v3
 *    2018/06/13 [servos]分离原主程序中的舵机控制代码
 *  v2
 *    2018/06/04 增加[pins]接口，提供BSP选择
 *    2018/06/03 增加机械手电机控制
 *    2018/06/02 增加[pins_v1]
 *    2018/06/02 增加机械臂转台电机控制
 *    2018/06/02 [SerialServo]增加attach()方法
 *  v1
 *    2018/05/30 增加TPA16.16C传感器通讯协议
 *    2018/05/29 [Thread]逻辑优化，增加线程ID
 *    2018/05/29 [pins]文件重构
 *    2018/05/28 [TPA16x16]提供“非阻塞”数据更新
 *    2018/05/28 [pins]新的扩展板接口定义
 *
 *  SixWDCarSlaveV3
 *  v4
 *    2018/05/18 修正Thread和SerialServoBus中的错误
 *    2018/05/16 添加TPA16.16C传感器模块
 ******************************************************************************/
// Libraries
#include <Servo.h>
#include <PS2X_lib.h>
#include <SD.h> // for [FreeRam()].
//#include <utility/SdFatUtil.h> // this is more accurate, but it dosn't work on Slober.

// Project
// BSP
#include "pins.h"
// OS
#include "Thread.h"
// HAL
#include "CommandReceiver.h"

#include "Car6WD.h"
#include "SerialServo.h"
#include "TPA16x16.h"

// Modules
#include "module/servos.h"

/*******************Constants**************************************************/
// Debug
#define DEBUG
#define DEBUG_PS2
#define DEBUG_CMD
#define DEBUG_REPORT
#define DEBUG_RAM
//#define DEBUG_TPA

// fast switching
#ifndef DEBUG
# undef DEBUG_PS2
# undef DEBUG_CMD
# undef DEBUG_REPORT
# undef DEBUG_RAM
# undef DEBUG_TPA
#endif

// PS/2
#define PS2_ENABLE

//--------Servos--------------------------------------------------------------//
// State & Warning
const uint8_t  SERVO_TEMP_WARNING    = 60; // 'C
const uint8_t  SERVO_TEMP_ERROR      = 65; // 'C
const int16_t  SERVO_VOLTAGE_WARNING = 7400; // mV

//--------PS/2----------------------------------------------------------------//
const uint16_t PS2_NO_BUTTON = 0;
// Motor
const uint16_t PS2_MOTOR_UP    = PSB_PAD_UP;
const uint16_t PS2_MOTOR_DOWN  = PSB_PAD_DOWN;
const uint16_t PS2_MOTOR_LEFT  = PSB_PAD_LEFT;
const uint16_t PS2_MOTOR_RIGHT = PSB_PAD_RIGHT;
const uint16_t PS2_MOTOR_SPEED_INC = PSB_L1;
const uint16_t PS2_MOTOR_SPEED_DEC = PSB_L2;
const uint8_t MOTOR_SPEED_INC = 25;
const int8_t  STICK_ERROR     = 16;
// Servo
const uint16_t PS2_SERVO_7_UP   = PSB_SQUARE;
const uint16_t PS2_SERVO_7_DOWN = PSB_CIRCLE;
const uint16_t PS2_SERVO_1_UP   = PSB_R1;
const uint16_t PS2_SERVO_1_DOWN = PSB_R2;
const uint16_t PS2_SERVO_2_UP   = PSB_TRIANGLE;
const uint16_t PS2_SERVO_2_DOWN = PSB_CROSS;
const uint16_t PS2_SERVO_6_UP   = PS2_NO_BUTTON;
const uint16_t PS2_SERVO_6_DOWN = PS2_NO_BUTTON;
const uint16_t PS2_SERVO_SPEED_INC = PS2_NO_BUTTON;
const uint16_t PS2_SERVO_SPEED_DEC = PS2_NO_BUTTON;
// Reset
const uint16_t PS2_RESET = PSB_START;
// Functional Buttons
const uint16_t PS2_FN = PSB_SELECT;
const uint16_t PS2_FN_SERVO_5_UP   = PSB_SQUARE;
const uint16_t PS2_FN_SERVO_5_DOWN = PSB_CIRCLE;
const uint16_t PS2_FN_SERVO_6_UP   = PSB_R1;
const uint16_t PS2_FN_SERVO_6_DOWN = PSB_R2;
const uint16_t PS2_FN_SERVO_8_UP   = PSB_TRIANGLE;
const uint16_t PS2_FN_SERVO_8_DOWN = PSB_CROSS;
const uint16_t PS2_FN_SERVO_SPEED_INC = PSB_L1;
const uint16_t PS2_FN_SERVO_SPEED_DEC = PSB_L2;
// Button Helpers
const uint16_t PS2_MOTORS       = PS2_MOTOR_UP | PS2_MOTOR_DOWN | PS2_MOTOR_LEFT | PS2_MOTOR_RIGHT;
const uint16_t PS2_MOTOR_SPEEDS = PS2_MOTOR_SPEED_INC | PS2_MOTOR_SPEED_DEC;
const uint16_t PS2_SERVO_7      = PS2_SERVO_7_UP | PS2_SERVO_7_DOWN;
const uint16_t PS2_SERVO_1      = PS2_SERVO_1_UP | PS2_SERVO_1_DOWN;
const uint16_t PS2_SERVO_2      = PS2_SERVO_2_UP | PS2_SERVO_2_DOWN;
const uint16_t PS2_SERVO_6      = PS2_SERVO_6_UP | PS2_SERVO_6_DOWN;
const uint16_t PS2_SERVOS       = PS2_SERVO_1 | PS2_SERVO_2 | PS2_SERVO_6 | PS2_SERVO_7;
const uint16_t PS2_SERVO_SPEEDS = PS2_SERVO_SPEED_INC | PS2_SERVO_SPEED_DEC;
const uint16_t PS2_FN_SERVO_5   = PS2_FN_SERVO_5_UP | PS2_FN_SERVO_5_DOWN;
const uint16_t PS2_FN_SERVO_6   = PS2_FN_SERVO_6_UP | PS2_FN_SERVO_6_DOWN;
const uint16_t PS2_FN_SERVO_8   = PS2_FN_SERVO_8_UP | PS2_FN_SERVO_8_DOWN;
const uint16_t PS2_FN_SERVOS    = PS2_FN_SERVO_5 | PS2_FN_SERVO_6 | PS2_FN_SERVO_8;
const uint16_t PS2_FN_SERVO_SPEEDS = PS2_FN_SERVO_SPEED_INC | PS2_FN_SERVO_SPEED_DEC;

//--------Commands------------------------------------------------------------//
// Global
const char CMD_HEADERS[] = "MIRST";
const char CMD_DELIMITER = ',';
// [M]otor
const uint8_t CMD_MOTOR = 0;
// Get [I]nfomation
const uint8_t CMD_INFO = 1;
const uint8_t CMD_INFO_SERVO = 1;
// [R]eset
const uint8_t CMD_RESET = 2;
void (*const RESET)() = 0; // Reset 'function' (Set PC = 0 to reset)
// [S]ervo
const uint8_t CMD_SERVO = 3;
const uint8_t CMD_SERVO_STOP = 0;
const uint8_t CMD_SERVO_UP   = 1;
const uint8_t CMD_SERVO_DOWN = 2;
// [T]hermograph
const uint8_t CMD_THERMOGRAPH = 4;
const uint8_t CMD_THERMOGRAPH_OFF = 0;
const uint8_t CMD_THERMOGRAPH_ON  = 1;

// Host - Report
const char REPORT_END = '\xFF';

//--------Thermograph---------------------------------------------------------//
const uint32_t TPA_TIMEOUT = 20; // ms, from its document.

/*******************Macros*****************************************************/
// Serial Debugging
#ifdef DEBUG
// to reduce program size, we use constant instead of 'F("...")'.
const char DEBUG_PROMPT_RX[] PROGMEM = " >> ";
const char DEBUG_PROMPT_TX[] PROGMEM = " << ";
# define debug(...)   (Debug.print(__VA_ARGS__))
# define debugS(s)    (Debug.print(F(s)))
# define debugln(...) (Debug.println(__VA_ARGS__))
# define debuglnS(s)  (Debug.println(F(s)))
// reinterpret_cast<const __FlashStringHelper *>: From [WString.h].
# define debugPromptRx() (debugTime(), debug(reinterpret_cast<const __FlashStringHelper *>(DEBUG_PROMPT_RX)))
# define debugPromptTx() (debugTime(), debug(reinterpret_cast<const __FlashStringHelper *>(DEBUG_PROMPT_TX)))
#else /* !DEBUG */
# define debug(...)
# define debugS(...)
# define debugln(...)
# define debugPromptRx()
# define debugPromptTx()
#endif /* DEBUG */

// Report
#ifdef DEBUG_REPORT
  const char DEBUG_REPORT_END[] PROGMEM = "\nReport End.\n";
# define report(...)  (Host.print(__VA_ARGS__), debug(__VA_ARGS__))
# define reportS(__s) (Host.print(F(__s)),      debugS(__s))
# define reportEnd()  (Host.print(REPORT_END),  debugln(reinterpret_cast<const __FlashStringHelper *>(DEBUG_REPORT_END)))
#else /* !DEBUG_REPORT */
# define report(...)  (Host.print(__VA_ARGS__))
# define reportS(__s) (Host.print(F(__s)))
# define reportEnd()  (Host.print(REPORT_END))
#endif /* DEBUG_REPORT */

// Range
#define In_Range(x, min, max) ((x) >= (min) && (x) <= (max))
#define In_Error_At_Ref(x, ref, error) In_Range(x, ref - error, ref + error)
#define In_Error(x, error) In_Error_At_Ref(x, 0, error)

/*******************Variables**************************************************/
//--------Threads-------------------------------------------------------------//
void mainThreadRun();
Thread mainThread(mainThreadRun, 0);
void ps2ThreadRun();
Thread ps2Thread(ps2ThreadRun, 50);
void servoInfoThreadRun();
Thread servoInfoThread(servoInfoThreadRun, 0);
void thermographThreadRun();
Thread thermographThread(thermographThreadRun, 500);

//--------Command Receiver----------------------------------------------------//
CommandReceiver cmd(Host, CMD_HEADERS, CMD_DELIMITER);
#ifdef DEBUG
CommandReceiver debugCmd(Debug, CMD_HEADERS, CMD_DELIMITER);
#endif /* DEBUG */

//--------Car Motion----------------------------------------------------------//
Car6WD car;

//--------PS/2----------------------------------------------------------------//
#ifdef PS2_ENABLE
PS2X ps2;
int16_t ps2Sticks[4]     = { 0 };
int16_t ps2SticksLast[4] = { 0 };
bool isStickLast        = false;
bool shouldStopMotors   = true;

uint8_t ps2MotorSpeed1 = 128;
uint8_t ps2MotorSpeed2 = 255;
uint8_t ps2ServoSpeed  = 3;
#endif /* PS2_ENABLE */

//--------TPA 16.16C Sensor---------------------------------------------------//
TPA16x16 tpa;
float *tpaFrame;
float tpaAmbientTemp = 0;

/*********Setup****************************************************************/
void setup() {
  delay(20);

  Host.begin(115200);
#ifdef DEBUG
  // we want Slober to identify serial settings and open Serial Monitor
  // automatically, so we disabled the following sentence,
  //Debug.begin(115200);
  // and use this instead.
  Serial.begin(115200);
#endif /* DEBUG */

  car.begin();

  servos_begin();

  tpa.begin();
  tpa.setTimeout(TPA_TIMEOUT);

  mainThread.start();

#ifdef PS2_ENABLE
  int8_t error = ps2.config_gamepad(PS2_CLK, PS2_CMD, PS2_CS, PS2_DAT);
  if (error != 0) {
    // try again later.
    delay(100);
    error = ps2.config_gamepad(PS2_CLK, PS2_CMD, PS2_CS, PS2_DAT);
  }
# ifdef DEBUG
  if (error == 0) {
    debugS("Found Controller, configured successful.\n");
    ps2Thread.start();
  } else if (error == 1) {
    debugS("No controller found, check wiring, see readme.txt to enable");
    debuglnS("debug. visit www.billporter.info for troubleshooting tips.");
  } else if (error == 2) {
    debugS("Controller found but not accepting commands. see readme.txt");
    debugS("to enable debug. Visit www.billporter.info for troubleshooting");
    debuglnS("tips.");
    ps2Thread.start();
  } else if (error == 3) {
    debuglnS("Controller refusing to enter Pressures mode, may not support it.");
    ps2Thread.start();
  }
# endif /* DEBUG */

  if (error != 1 && (ps2.readType() == 0 || ps2.readType() == 2)) {
# ifdef DEBUG
    debugS("This controller is not support the device. please use");
    debugS("Wireless Sony DualShock Controller.\n");
# endif /* DEBUG */
  }

#endif /* PS2_ENABLE */

  servos_reset();

  delay(100);

  debugPromptRx();
  reportS("Ready.\n");
  reportEnd();
}

/*******************Loop*******************************************************/
//--------Main Thread---------------------------------------------------------//
void mainThreadRun() {
  // handlers running.
  yield();

  //if (serialEventRun) serialEventRun();
}

//--------Handlers (Events)---------------------------------------------------//
// Handler 1 - Upper Computer Command Processing
void serialEvent2() {
  cmd.listener();
  // multi-thread feature.
  if (cmd.available()) processCommand(cmd);
}

// Debugging Handler - Same as Handler 1
#ifdef DEBUG
void serialEvent() {
  debugCmd.listener();
  if (debugCmd.available()) processCommand(debugCmd);
}
#endif

inline void processCommand(CommandReceiver &cmd) {
  uint8_t *str;
  cmd.peek(str);

  debugPromptRx();
  debugln((char* )str);

  uint8_t header = cmd.read(CMD_FIELD_HEAD);
  uint16_t parm1 = cmd.read(CMD_FIELD_PARM(1));
  uint16_t parm2 = cmd.read(CMD_FIELD_PARM(2));
  uint16_t parm3 = cmd.read(CMD_FIELD_PARM(3));

  // start with the header 'M' ?
  if (header == CMD_MOTOR) {
    car.run((int16_t) parm1, (int16_t) parm2);
  }
  // start with the header 'I' ?
  else if (header == CMD_INFO) {
    if (parm1 == CMD_INFO_SERVO) {
      servoInfoThread.start();
      // remember to remove the command from the buffer.
      cmd.read();
      // because we will finish the report in [servoInfoThread()].
      return;
    }
    else goto fail;
  }
  // start with the header 'R' ?
  else if (header == CMD_RESET) {
    // wait for transmitting finish.
    delay(5);
    // we want to check the booting of device, and it will send a
    // report when it boot successfully, so we'll wait instead of
    // send the report immediately.
    RESET();
    // unreachable
  }
  // start with the header 'S' ?
  else if (header == CMD_SERVO) {
    // make sure that the index is valid,
    if (parm1 < 1 || parm1 > NUM_OF_ALL_SERVOS) goto fail;
    // and speed isn't out of index.
    if (parm3 > NUM_OF_SERVO_SPEEDS) goto fail;

         if (parm2 == CMD_SERVO_STOP) servos_run(parm1,  0);
    else if (parm2 == CMD_SERVO_UP)   servos_run(parm1,  (int8_t)parm3);
    else if (parm2 == CMD_SERVO_DOWN) servos_run(parm1, -(int8_t)parm3);
    else goto fail;
  }
  // start with the header 'T' ?
  else if (header == CMD_THERMOGRAPH) {
         if (parm1 == CMD_THERMOGRAPH_ON)  thermographThread.start();
    else if (parm1 == CMD_THERMOGRAPH_OFF) thermographThread.stop();
    else goto fail;
  }
  // unsupported function.
  else goto fail;

/* success: */
  debugPromptTx();
  reportS("OK.");
  reportEnd();

  cmd.read();
  return;

fail:
  debugPromptTx();
  reportS("Bad command.");
  reportEnd();

  cmd.read();
}

//--------Threads-------------------------------------------------------------//
#ifdef PS2_ENABLE
void ps2ThreadRun() {
  ps2.read_gamepad();
  // backup last state.
  memcpy(ps2SticksLast, ps2Sticks, sizeof(ps2Sticks));
  ps2Sticks[0] = (int16_t) ps2.Analog(PSS_LX) - 128;
  ps2Sticks[1] = (int16_t) 128 - ps2.Analog(PSS_LY);
  ps2Sticks[2] = (int16_t) ps2.Analog(PSS_RX) - 128;
  ps2Sticks[3] = (int16_t) 128 - ps2.Analog(PSS_RY);

  if (ps2.Button(PS2_MOTORS)) {
    // motor buttons.
    if (ps2.Button(PS2_MOTOR_UP))    car.run( ps2MotorSpeed1,  ps2MotorSpeed1);
    if (ps2.Button(PS2_MOTOR_DOWN))  car.run(-ps2MotorSpeed1, -ps2MotorSpeed1);
    if (ps2.Button(PS2_MOTOR_LEFT))  car.run(-ps2MotorSpeed1,  ps2MotorSpeed1);
    if (ps2.Button(PS2_MOTOR_RIGHT)) car.run( ps2MotorSpeed1, -ps2MotorSpeed1);
    isStickLast = false;
  } else {
    // motor stick.
    if (!In_Error(ps2Sticks[0], STICK_ERROR) || !In_Error(ps2Sticks[1], STICK_ERROR)) {
      double therta = atan2(-ps2Sticks[1], ps2Sticks[0]) - M_PI_4;
      double r = hypot(ps2Sticks[1], ps2Sticks[0]) * ps2MotorSpeed1 * 0.01111;
      car.run(-sin(therta) * r, -cos(therta) * r);

      isStickLast = true;

#ifdef DEBUG_PS2
      {
        debugS("Stick Values: ");
        debug(ps2Sticks[0], DEC);
        debugS(",\t");
        debug(ps2Sticks[1], DEC);
        debugS(",\t");
        debug(ps2Sticks[2], DEC);
        debugS(",\t");
        debugln(ps2Sticks[3], DEC);
      }
#endif /* DEBUG_PS2 */

    } else {
      // if we get here, none of motor buttons is pressing, or any motor sticks
      // is released.
      if (ps2.ButtonReleased(PS2_MOTORS)) car.stop(255, 255);
      if (isStickLast) car.run(1, 1);
      isStickLast = false;
    } /* if */

  } /* if (ps2.Button(PS2_MOTORS)) */

  // motor speed buttons.
  if (ps2.ButtonPressed(PS2_MOTOR_SPEED_INC))
    if (ps2MotorSpeed1 + MOTOR_SPEED_INC <= 255) ps2MotorSpeed1 += MOTOR_SPEED_INC;
  if (ps2.ButtonPressed(PS2_MOTOR_SPEED_DEC))
    if (ps2MotorSpeed1 - MOTOR_SPEED_INC >= 0)   ps2MotorSpeed1 -= MOTOR_SPEED_INC;

  if (!ps2.Button(PS2_FN)) {
    // servo buttons.
    if (ps2.Button(PS2_SERVO_1_UP))      servos_run(1,  ps2ServoSpeed);
    if (ps2.Button(PS2_SERVO_1_DOWN))    servos_run(1, -ps2ServoSpeed);
    if (ps2.ButtonReleased(PS2_SERVO_1)) servos_run(1, 0);
    if (ps2.Button(PS2_SERVO_2_UP))      servos_run(2,  ps2ServoSpeed);
    if (ps2.Button(PS2_SERVO_2_DOWN))    servos_run(2, -ps2ServoSpeed);
    if (ps2.ButtonReleased(PS2_SERVO_2)) servos_run(2, 0);
    if (ps2.Button(PS2_SERVO_6_UP))      servos_run(6,  ps2ServoSpeed);
    if (ps2.Button(PS2_SERVO_6_DOWN))    servos_run(6, -ps2ServoSpeed);
    if (ps2.ButtonReleased(PS2_SERVO_6)) servos_run(6, 0);
    if (ps2.Button(PS2_SERVO_7_UP))      servos_run(7,  ps2ServoSpeed);
    if (ps2.Button(PS2_SERVO_7_DOWN))    servos_run(7, -ps2ServoSpeed);
    if (ps2.ButtonReleased(PS2_SERVO_7)) servos_run(7, 0);

    // servo speed buttons.
    /*
    if (ps2.ButtonPressed(PS2_SERVO_SPEED_INC))
      if (ps2ServoSpeed < NUM_OF_SERVO_SPEEDS - 1) ps2ServoSpeed ++;
    if (ps2.ButtonPressed(PS2_SERVO_SPEED_DEC))
      if (ps2ServoSpeed > 0) ps2ServoSpeed --;
    */

    // servo sticks.
         if (ps2Sticks[3] < -64) servos_run(3,  ps2ServoSpeed);
    else if (ps2Sticks[3] >  63) servos_run(3, -ps2ServoSpeed);
    else if (!In_Range(ps2SticksLast[3], -64, 63) && In_Range(ps2Sticks[3], -64, 63)) servos_run(3, 0);
         if (ps2Sticks[2] < -64) servos_run(4,  ps2ServoSpeed);
    else if (ps2Sticks[2] >  63) servos_run(4, -ps2ServoSpeed);
    else if (!In_Range(ps2SticksLast[2], -64, 63) && In_Range(ps2Sticks[2], -64, 63)) servos_run(4, 0);

  } else { /* ps2.Button(PS2_FN) */

    if (ps2.Button(PS2_FN_SERVO_5_UP))      servos_run(5,  ps2ServoSpeed);
    if (ps2.Button(PS2_FN_SERVO_5_DOWN))    servos_run(5, -ps2ServoSpeed);
    if (ps2.ButtonReleased(PS2_FN_SERVO_5)) servos_run(5, 0);
    if (ps2.Button(PS2_FN_SERVO_6_UP))      servos_run(6,  ps2ServoSpeed);
    if (ps2.Button(PS2_FN_SERVO_6_DOWN))    servos_run(6, -ps2ServoSpeed);
    if (ps2.ButtonReleased(PS2_FN_SERVO_6)) servos_run(6, 0);
    if (ps2.Button(PS2_FN_SERVO_8_UP))      servos_run(8,  ps2ServoSpeed);
    if (ps2.Button(PS2_FN_SERVO_8_DOWN))    servos_run(8, -ps2ServoSpeed);
    if (ps2.ButtonReleased(PS2_FN_SERVO_8)) servos_run(8, 0);

    // servo speed buttons.
    if (ps2.ButtonPressed(PS2_FN_SERVO_SPEED_INC))
      if (ps2ServoSpeed < NUM_OF_SERVO_SPEEDS - 1) ps2ServoSpeed ++;
    if (ps2.ButtonPressed(PS2_FN_SERVO_SPEED_DEC))
      if (ps2ServoSpeed > 0) ps2ServoSpeed --;
  } /* !ps2.Button(PS2_FN) */

  if (ps2.ButtonPressed(PS2_FN) || ps2.ButtonReleased(PS2_FN)) {
    servos_run(1, 0);
    servos_run(2, 0);
    servos_run(3, 0);
    servos_run(4, 0);
    servos_run(5, 0);
    servos_run(6, 0);
    servos_run(7, 0);
    servos_run(8, 0);
  }

  // for debug?
  if (ps2.ButtonPressed(PSB_R3)) {
    if (!thermographThread.isAlive())
      thermographThread.start();
    else
      thermographThread.stop();
  }

#ifdef DEBUG_PS2
  {
    if (ps2.ButtonPressed(PS2_MOTORS | PS2_MOTOR_SPEEDS)) {
      debugS("Motor speed: ");
      debug(ps2MotorSpeed1, DEC);
      debugS(",\t");
      debugln(ps2MotorSpeed2, DEC);
    }
    if ((!ps2.Button(PS2_FN) && ps2.ButtonPressed(PS2_SERVOS | PS2_SERVO_SPEEDS)) ||
        ( ps2.Button(PS2_FN) && ps2.ButtonPressed(PS2_FN_SERVOS | PS2_FN_SERVO_SPEEDS))) {
      debugS("Servo speed: ");
      debugln(ps2ServoSpeed, DEC);
    }
  }
#endif /* DEBUG_PS2 */

  if (ps2.ButtonPressed(PS2_RESET)) {
#ifdef DEBUG_PS2
    {
      debugS("Reset!\n");
      delay(1);
    }
#endif /* DEBUG_PS2 */
    RESET();
    // unreachable

  } /* ps2.ButtonPressed(PS2_RESET) */
}
#endif /* PS2_ENABLE */

void servoInfoThreadRun() {
  // up-link command format:
  // I1,<ID>,<Temperature>[N|W|E],<Voltage>[N|E],<GoalPos>,<CurrentPos>[N]

  int16_t val;
  for (uint8_t id = 1; id <= NUM_OF_LOCABLE_SERVOS; id ++) {
    debugPromptTx();
    reportS("I1");

    report(',');
    report(id, DEC);

    report(',');
    val = servos_getTemperature(id);
    report(val, DEC);
    if (val == SerialServo::FAIL_VALUE) report('N'); // 'N' means 'NaN', no data.
    else if (val >= SERVO_TEMP_ERROR) report('E');
    else if (val >= SERVO_TEMP_WARNING) report('W');

    report(',');
    val = servos_getVoltage(id);
    report(val, DEC);
    if (val == SerialServo::FAIL_VALUE) report('N');
    else if (val <= SERVO_VOLTAGE_WARNING) report('W');

    report(',');
    report(servos_getGoalPos(id), DEC);

    report(',');
    val = servos_getCurrentPos(id);
    report(val, DEC);
    if (val == SerialServo::FAIL_VALUE) report('N');

    report('\n');
  }
  // we also send '\n' just now.
  debugPromptTx();

#ifdef DEBUG_RAM
  {
    debugS("\nFree Ram: ");
    debug(FreeRam(), DEC);
    debugS(" Bytes");
  }
#endif /* DEBUG_RAM */

  reportEnd();

  // this thread always runs once when a call.
  servoInfoThread.stop();
}

void thermographThreadRun() {
  tpa.updateData();
  tpaFrame = tpa.getPrePorcessedFrame(tpaAmbientTemp);

#ifdef DEBUG_TPA
  {
    debugTPAFrame();
  }
#else /* !DEBUG_TPA */
  {
    // up-link command format:
    // T0,<ambient temperature, 1 digit>
    debugPromptTx();
    reportS("T0,");
    report(tpaAmbientTemp, 1);

    // up-link command format:
    // T<row number>,<cloumn 1>,<cloumn 2>,...,<cloumn 16>
    for (uint16_t i = 0; i < TPA16x16::NUMBER_OF_PIXELS; i ++) {
      // 16 pixels (1 row) per line.
      if ((i & 0xF) == 0) {
        report('\n');

        // it takes a long time (>100ms) to report, so we should do some
        // listening like multi-threading.
        yield();
        if (serialEventRun) serialEventRun();

        debugPromptTx();
        report('T');
        report((i >> 4) + 1, DEC); // instead of 'i / 16'
      }

      report(',');
      report(tpaFrame[i], 0);
    }
    report('\n');
    reportEnd();
  }
#endif /* DEBUG_TPA */
}

//--------Other Functions-----------------------------------------------------//
#ifdef DEBUG_CMD
void debugTime() {
  uint32_t t = millis();
  uint8_t mm = t / 60000;
  uint8_t ss = t / 1000 % 60;
  uint16_t zzz = t % 1000;

  debug('[');
  // format digital bits.
  if (mm < 10) debug('0');
  debug(mm, DEC);
  debug(':');
  if (ss < 10) debug('0');
  debug(ss, DEC);
  debug('.');
  if (zzz < 100) debug('0');
  if (zzz < 10) debug('0');
  debug(zzz, DEC);
  debug(']');
}
#endif /* DEBUG_CMD */

inline void debugTPAFrame() {
  debugln();
  debugln();
  debugln();

  // find the hottest and coldest point in the frame.
  float minTemp = INFINITY;
  float maxTemp = -INFINITY;
  for (uint16_t i = 0; i < TPA16x16::NUMBER_OF_PIXELS; i ++) {
    // the sensor has reported that some problem was in this pixel,
    // and it has very different data from the others.
    if (i == 237) continue;

    if (tpaFrame[i] < minTemp) minTemp = tpaFrame[i];
    if (tpaFrame[i] > maxTemp) maxTemp = tpaFrame[i];
  }

  // print pixels as a ASCII art.
  for (uint16_t i = 0; i < TPA16x16::NUMBER_OF_PIXELS; i ++) {
    float ratio = (tpaFrame[i] - minTemp) / (maxTemp - minTemp);

         if (ratio < .125) debugS("  ");
    else if (ratio < .250) debugS("..");
    else if (ratio < .375) debugS(";;");
    else if (ratio < .500) debugS("oo");
    else if (ratio < .625) debugS("OO");
    else if (ratio < .750) debugS("00");
    else if (ratio < .875) debugS("%%");
    else                   debugS("@@");

    if ((i & 0xF) == 0xF) debugln();
  }
  debugln();

  // the more information.
  debug(pow(maxTemp * 1e7f, 0.25) - 273, 1);
  debugS("C\t@ ");
  debug(tpaAmbientTemp, 1);
  debuglnS("C\n | ");
  debug(pow(minTemp * 1e7f, 0.25) - 273, 1);
  debuglnS("C");
}

/******************************************************************************/
