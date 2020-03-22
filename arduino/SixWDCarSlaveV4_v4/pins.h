/*******************************************************************************
 * File name: pins.h
 * Created on: 2018/06/04 08:12:57
 * Author: Wuxy
 * BSP接口
 * 
 * History:
 * 	v1
 * 		2018/06/04 创建文件
*******************************************************************************/
#ifndef PINS_H_
#define PINS_H_

#define EXTEND_BOARD_VERSION 2

#if   EXTEND_BOARD_VERSION == 1
#include "pins_v1.h"
#elif EXTEND_BOARD_VERSION == 2
#include "pins_v2.h"
#else
#error the value of 'EXTEND_BOARD_VERSION' is invalid.
#endif

#endif /* PINS_H_ */
