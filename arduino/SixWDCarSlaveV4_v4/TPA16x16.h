/*******************************************************************************
 * File name: TPA16x16.h
 * Source file: TPA16x16.cpp
 * Used libraries: SPI
 * Created on: 2018年5月16日
 * Author: Wuxy
 * TPA 16.16C 5572 L4.7 模块 SPI通信库
 *
 * History:
 *  v13
 *    2018-05-30 [source file]调整帧预处理参数
 *    2018-05-28 增加[setTimeout(uint32_t)]，提供“非阻塞”数据更新
 ******************************************************************************/
#ifndef TPA16X16_H_
#define TPA16X16_H_

#include <Arduino.h>
// Libraries
#include <SPI.h>

class TPA16x16 {
  public:
    static const uint16_t NUMBER_OF_ROWS    = 16;
    static const uint16_t NUMBER_OF_CLOUMNS = 16;
    static const uint16_t NUMBER_OF_PIXELS  = NUMBER_OF_ROWS * NUMBER_OF_CLOUMNS;
    static const uint8_t  NUMBER_OF_PARMS   = 45;

  protected:
    uint32_t mTimeout;

    uint8_t  mStatus;
    uint16_t mBuffer[NUMBER_OF_PIXELS];
    float    mAmbientTemp;
    float    mPixels[NUMBER_OF_PIXELS];

  public:
    TPA16x16();
    void begin();
    void setTimeout(uint32_t timeout);

    uint8_t readStatus();
    bool avaliable();
    bool updateData();

    float getAmbientTemperature();
    float getPixel(uint8_t x, uint8_t y);
    float* getFrame(float &ambientTemp);
    float* getPrePorcessedFrame(float &ambientTemp);
    uint16_t* getRawFrame();

    float     readAmbientTemperature();
    uint16_t* readPixelOffset();
    uint16_t* readPixelSensitivity();
    uint16_t  readParameter(uint16_t index);
    uint16_t* readAllParameter();

    void end();
};

#endif /* TPA16X16_H_ */
