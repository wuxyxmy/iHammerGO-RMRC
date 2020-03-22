/*******************************************************************************
 * File name: main.js
 * Created on: 2018/02/21
 * 遥控界面核心脚本
 * 
 * History:
 *  v1.0
 *    2019/06/30 添加二维码识别服务控制事件
 *    2019/06/30 同步电机速度控制方法，与下位机保持一致
 *  v0.9
 *    2018/06/19 添加热像仪按钮事件
 *  v0.8
 *    2018/06/12
 *  v0.7
 *    2018/06/04 优化按钮事件逻辑与jQuery选择器的使用
 *    2018/06/03 优化键盘事件逻辑
 *    2018/05/31 文件重构
*******************************************************************************/
/******************* Constants ************************************************/
// Debug
const DEBUG     = true;
const DEBUG_BTN = true;

// Video Streams
const NUM_VIDEO_STREAMS = 2;
const VIDEO_STREAM_IDS = ["#stream-video-1", "#stream-video-2"];
const VIDEO_STREAM_PORTS = [8080, 8081];

const BARCODE_STREAM_PORT = 8082;

//  Connection
const SLAVE_TIMEOUT = 1100; // ms

// Keyboard Shortcut
const KEY_TO_BTN_MAP = {
  "F2" : "#btn-reset", "F4" : "#btn-servo-get-info",

  "I" : "#btn-motor-forward",        "J" : "#btn-motor-left",
  "K" : "#btn-motor-backward",       "L" : "#btn-motor-right",
  "U" : "#btn-motor-speed-increase", "O" : "#btn-motor-speed-decrease",

  "Q" : "#btn-servo-1-up",           "A" : "#btn-servo-1-down",
  "W" : "#btn-servo-2-up",           "S" : "#btn-servo-2-down",
  "E" : "#btn-servo-3-up",           "D" : "#btn-servo-3-down",
  "R" : "#btn-servo-4-up",           "F" : "#btn-servo-4-down",
  "T" : "#btn-servo-6-up",           "G" : "#btn-servo-6-down",

  "B" : "#btn-servo-7-up",           "N" : "#btn-servo-7-down"
};

//---------- Passive ---------------------------------------------------------//
// Debug - Passive
const D  = !!DEBUG;
const DB = D && !!DEBUG_BTN;

// Mouse Events
const BTN_EVENT = {
  "INVAILD"  : -1,
  "ACTIVE"   :  0,
  "INACTIVE" :  1
};
const BTN_MOUSE_EVENT_MAP = {
  "mousedown"  : BTN_EVENT.ACTIVE,
  "mouseup"    : BTN_EVENT.INACTIVE,
  "mouseleave" : BTN_EVENT.INACTIVE
};
// because jQuery selector dose not work well before the document
// ready, we have to set it later, but its behaver still likes a
// constant.
var NUM_BUTTONS = 0;

// Keyboard Button Control
const BTN_KEY_EVENT_MAP = {
  "keydown" : BTN_EVENT.ACTIVE,
  "keyup"   : BTN_EVENT.INACTIVE
};

// Optimization of key events
var KEYCODE_TO_BTN_MAP = new Array(KEYCODE_MAX + 1);
// one key can only bind one button, so we only need look up the
// keycode table.
for (var key in KEYCODES) {
  if (KEY_TO_BTN_MAP[key] !== undefined) {
    KEYCODE_TO_BTN_MAP[KEYCODES[key]] = KEY_TO_BTN_MAP[key];
  }
}

// Slave Commands
const CMD_MOTOR      = "M";
const CMD_RESET      = "R";
const CMD_SERVO      = "S";
const CMD_SERVO_INFO = "I1";

const NUMBER_OF_MOTOR_SPEEDS = 10;
const MOTOR_SPEED_MAP = [
  // 30, 64 ~ 128, 128 ~ 256, log
  30, 64, 76, 91, 108, 128, 147, 194, 223, 255
];

const CMD_SERVO_STOP = 0;
const CMD_SERVO_UP   = 1;
const CMD_SERVO_DOWN = 2;
const NUMBER_OF_SERVO_SPEEDS = 5;
const CMD_SERVO_SPEED_MAP = [0, 1, 2, 3, 4];

// Servo Information
const MILLIS_TASK_SERVOINFO       = 2000; // ms
const MILLIS_TASK_SERVOINFO_BLOCK = SLAVE_TIMEOUT + 100; // ms

// Elements & Display
var motorSpeedDisplay  = null;
var servoSpeedDisplay  = null;
var statusBar          = null;
var servoStateArea     = null;
var thermographFrame   = null;
const SERVO_SPEED_NAME = ["Low", "Mid-Low", "Middle", "Mid-High", "High"];

/******************* Variables ************************************************/
// because button event happens on button's state changes, but
// 'keydown' event may be triggered continually while the key is
// holding, we must store last key states to trig button events
// at right time.
var lastKeyStates = new Array(KEYCODE_MAX + 1);
var lastButtonStates = {};

var server = new Server();

var connectionCount = 0;

var motorSpeedLevel = 5;
var servoSpeed = NUMBER_OF_SERVO_SPEEDS - 2; // (3) Mid-High

var servoInfoTask = null;

/******************* Events ***************************************************/
//---------- Page Ready ------------------------------------------------------//
$(document).ready(function () {
  // 'constant' initialize
  const buttons = $("button");
  NUM_BUTTONS = buttons.length;
  for (var i = 0; i < NUM_BUTTONS; i ++) {
    const btnId = "#" + buttons[i].id;
    lastButtonStates[btnId] = BTN_EVENT.INACTIVE;
  }

  motorSpeedDisplay = $("#dropdown-motor-speed");
  servoSpeedDisplay = $("#dropdown-servo-speed");
  statusBar         = $("#status-bar");
  servoStateArea    = $("#status-servos");
  thermographFrame  = $("#frame-thermograph");
  barcodeReaderStream = $("#stream-barcode-reader");
  
  // page element initialize
  for (var i = 0; i < NUM_VIDEO_STREAMS; i ++) {
    $(VIDEO_STREAM_IDS[i]).attr("src", "http://" + window.location.hostname
      + ":" + VIDEO_STREAM_PORTS[i] + "/?action=stream");
  }

  for (var eType in BTN_MOUSE_EVENT_MAP) {
    $("button").on(eType, function (e) {
      dispatchButtonEvent("#" + this.id, BTN_MOUSE_EVENT_MAP[e.type]);
    });
  }
  
  for (var eType in BTN_KEY_EVENT_MAP) {
    $("body").on(eType, function (e) {
      dispatchKeyEvent(e.keyCode, e.type);
    });
  }

  motorSpeedChangedEvent();
  servoSpeedChangedEvent();

  //server.load("I1", $("server_command"));
});

//---------- Button Events ---------------------------------------------------//
function dispatchButtonEvent(selector, type) {
  if (lastButtonStates[selector] == type) return;
  lastButtonStates[selector] = type;
  
  switch (selector) {
    case "#btn-reset":
    case "#btn-servo-get-info":
    case "#btn-exit":
    case "#btn-shutdown":
    case "#btn-motor-speed-increase":
    case "#btn-motor-speed-decrease":
    case "#btn-servo-speed-increase":
    case "#btn-servo-speed-decrease":
    case "#btn-thermograph-on":
    case "#btn-thermograph-off":
    case "#btn-barcode-reader-on":
    case "#btn-barcode-reader-on-zbarlight":
    case "#btn-barcode-reader-off":
      dispatchOpreationButtonEvent(selector, type);
      return;
    
    case "#btn-motor-forward":
    case "#btn-motor-left":
    case "#btn-motor-backward":
    case "#btn-motor-right":
      dispatchMotorButtonEvent(selector, type);
      return;
    
    case "#btn-servo-1-up":
    case "#btn-servo-1-down":
    case "#btn-servo-2-up":
    case "#btn-servo-2-down":
    case "#btn-servo-3-up":
    case "#btn-servo-3-down":
    case "#btn-servo-4-up":
    case "#btn-servo-4-down":
    case "#btn-servo-5-up":
    case "#btn-servo-5-down":
    case "#btn-servo-6-up":
    case "#btn-servo-6-down":
    case "#btn-servo-7-up":
    case "#btn-servo-7-down":
      dispatchServoButtonEvent(selector, type);
      return;
      
    default:
      return;
  }
}

function dispatchOpreationButtonEvent(selector, type) {
  // Only complete clicking is able to make a operation.
  if (type != BTN_EVENT.ACTIVE) return;
  
  switch (selector) {
    case "#btn-reset":
      if (confirm("Do you really want to reset?"))
        executeCommand(CMD_RESET);
      break;
    case "#btn-servo-get-info":
      if (servoInfoTask == null) {
        startServoInfoTask();
        $(selector).style.backgroundColor = "#40FF40";
      } else {
        stopServoInfoTask();
        servoInfoTask = null;
        $(selector).style.backgroundColor = "#F0F0F0";
      }
      break;
    
    case "#btn-exit":
      if (confirm("Do you really want to exit?"))
        httpRequest("exit");
      break;
    case "#btn-shutdown":
      if (confirm("Do you really want to shutdown?"))
        httpRequest("shutdown");
      break;
    
    case "#btn-motor-speed-increase":
      if (motorSpeedLevel < NUMBER_OF_MOTOR_SPEEDS - 1) {
        motorSpeedLevel ++;
        motorSpeedChangedEvent();
      }
      break;
    case "#btn-motor-speed-decrease":
      if (motorSpeedLevel > 0) {
        motorSpeedLevel --;
        motorSpeedChangedEvent();
      }
      break;
    
    case "#btn-servo-speed-increase":
      if (servoSpeed < NUMBER_OF_SERVO_SPEEDS - 1) {
        servoSpeed ++;
        servoSpeedChangedEvent();
      }
      break;
    case "#btn-servo-speed-decrease":
      if (servoSpeed > 0) {
        servoSpeed --;
        servoSpeedChangedEvent();
      }
      break;

    case "#btn-thermograph-on":
      server.execute("T1");
      thermographFrame.attr("src", "/thermograph");
      break;
    case "#btn-thermograph-off":
      thermographFrame.attr("src", "about:blank");
      server.execute("T0");
      break;

    case "#btn-barcode-reader-on-zbarlight":
      var method = "zbarlight";
    case "#btn-barcode-reader-on":
      server.barcodeReaderControl(true, method || "pyzbar");
      barcodeReaderStream.attr(
          "src",
          "http://" + window.location.hostname
            + ":" + BARCODE_STREAM_PORT + "/?action=stream");
      break;
    case "#btn-barcode-reader-off":
      barcodeReaderStream.attr("src", " ");
      server.barcodeReaderControl(false);
      break;
    
    default:
      break;
  }
}

function dispatchMotorButtonEvent(selector, type) {
  switch (type) {
    case BTN_EVENT.ACTIVE:
      const speed = MOTOR_SPEED_MAP[motorSpeedLevel];
      switch (selector) {
        case "#btn-motor-forward":
          executeCommand(CMD_MOTOR + speed + "," + speed);
          break;
        case "#btn-motor-left":
          executeCommand(CMD_MOTOR + "-" + speed + "," + speed);
          break;
        case "#btn-motor-backward":
          executeCommand(CMD_MOTOR + "-" + speed + ",-" + speed);
          break;
        case "#btn-motor-right":
          executeCommand(CMD_MOTOR + speed + ",-" + speed);
          break;
        
        default:
          break;
      }
      break;
    
    case BTN_EVENT.INACTIVE:
      executeCommand(CMD_MOTOR + "0,0");
      break;
    
    default:
      break;
  }
}

function dispatchServoButtonEvent(selector, type) {
  var id = 0;
  var action = CMD_SERVO_STOP;
  
  switch (selector) {
    case "#btn-servo-1-up":
      id = 1; action = CMD_SERVO_DOWN; break;
    case "#btn-servo-1-down":
      id = 1; action = CMD_SERVO_UP;   break;
    case "#btn-servo-2-up":
      id = 2; action = CMD_SERVO_DOWN; break;
    case "#btn-servo-2-down":
      id = 2; action = CMD_SERVO_UP;   break;
    case "#btn-servo-3-up":
      id = 3; action = CMD_SERVO_DOWN; break;
    case "#btn-servo-3-down":
      id = 3; action = CMD_SERVO_UP;   break;
    case "#btn-servo-4-up":
      id = 4; action = CMD_SERVO_UP;   break;
    case "#btn-servo-4-down":
      id = 4; action = CMD_SERVO_DOWN; break;
    case "#btn-servo-5-up":
      id = 5; action = CMD_SERVO_UP;   break;
    case "#btn-servo-5-down":
      id = 5; action = CMD_SERVO_DOWN; break;
    case "#btn-servo-6-up":
      id = 8; action = CMD_SERVO_UP;   break;
    case "#btn-servo-6-down":
      id = 8; action = CMD_SERVO_DOWN; break;
    case "#btn-servo-7-up":
      id = 7; action = CMD_SERVO_UP;   break;
    case "#btn-servo-7-down":
      id = 7; action = CMD_SERVO_DOWN; break;
    default:
      break;
  }
  
  switch (type) {
    case BTN_EVENT.ACTIVE:   /* Keep before value */  break;
    case BTN_EVENT.INACTIVE: action = CMD_SERVO_STOP; break;
    default: break;
  }
  
  executeCommand(CMD_SERVO + id + "," + action + "," +
    CMD_SERVO_SPEED_MAP[servoSpeed]);
}

//---------- Key Events ------------------------------------------------------//
function dispatchKeyEvent(keyCode, type) {
  const selector = KEYCODE_TO_BTN_MAP[keyCode];
  
  // the key hasn't bound with any button.
  if (selector == undefined) return;
  
  // the same event trigger more than once.
  if (lastKeyStates[keyCode] == type) return;
  // update last state.
  lastKeyStates[keyCode] = type;
  // trigger the button.  
  dispatchButtonKeyEvent(selector, BTN_KEY_EVENT_MAP[type]);
}

function dispatchButtonKeyEvent(selector, type) {
  switch (type) {
    case BTN_EVENT.ACTIVE:
      $(selector).addClass("active");
      break;
    case BTN_EVENT.INACTIVE:
      $(selector).removeClass("active");
      break;
      
    default:
      break;
  }

  if(DB)console.log($(selector).attr("id"));

  dispatchButtonEvent(selector, type);
}

//---------- State & Display -------------------------------------------------//
function motorSpeedChangedEvent() {
  motorSpeedDisplay.text("Speed: " + MOTOR_SPEED_MAP[motorSpeedLevel]);
}
function servoSpeedChangedEvent() {
  servoSpeedDisplay.text("S: " + (servoSpeed + 1));
}

//---------- Server Interfaces -----------------------------------------------//
function executeCommand(cmd) {
  server.execute(cmd);
}

function startServoInfoTask() {
  // run in time first.
  servoInfoTaskCallback();
  servoInfoTask = setInterval(servoInfoTaskCallback, MILLIS_TASK_SERVOINFO);
}
function servoInfoTaskCallback() {
  if (connectionCount > 0) {
    // some connection are still running.
    setTimeout(servoInfoTaskCallback, MILLIS_TASK_SERVOINFO_BLOCK);
    
    if(D)console.log("Task: Waiting for free.");
  } else {
    //server.execute('I1');
    //server.load_info('I1', servoStateArea);
    httpRequest("info?cmd=" + CMD_SERVO_INFO,
      function(response) {
        servoStateArea.html(response);
      });
  }
}
function stopServoInfoTask() {
  clearInterval(servoInfoTask);
}

function httpRequest(url, callback) {
  // begin connecting.
  connectionCount ++;
  if(D)console.log("Connection: " + connectionCount + " Open +New");
  
  // before sending.
  statusBar.innerHTML = "Status: Buzy...";
  
  var httpConnection = new XMLHttpRequest();
  httpConnection.timeout = SLAVE_TIMEOUT;
  httpConnection.open("GET", url, true);
  httpConnection.onreadystatechange = function() {
    if (httpConnection.readyState == 4) {
      if (httpConnection.status == 200) {
        // succeed.
        statusBar.innerHTML = "Status: Ready.";
        // call the function 'callback'.
        callback(httpConnection.responseText);
      } else {
        // timeout or no connection.
        statusBar.innerHTML = "Status: Connection was lost.";
      }
      // finish connecting.
      connectionCount --;
      
      if(D)console.log("Connection: " + connectionCount + " Open -Finish.");
    }
  };
  httpConnection.send();
}

/******************************************************************************/
