#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2019-06-26 14:41:12

#include "Arduino.h"
#include <Servo.h>
#include <PS2X_lib.h>
#include <SD.h>
#include "pins.h"
#include "Thread.h"
#include "CommandReceiver.h"
#include "Car6WD.h"
#include "SerialServo.h"
#include "TPA16x16.h"
#include "module/servos.h"

void setup() ;
void mainThreadRun() ;
void serialEvent2() ;
void serialEvent() ;
inline void processCommand(CommandReceiver &cmd) ;
void ps2ThreadRun() ;
void servoInfoThreadRun() ;
void thermographThreadRun() ;
void debugTime() ;
inline void debugTPAFrame() ;

#include "SixWDCarSlaveV4_v4.ino"


#endif
