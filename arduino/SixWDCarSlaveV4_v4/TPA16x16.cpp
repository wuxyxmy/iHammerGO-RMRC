/*******************************************************************************
 * File name: TPA16x16.cpp
 * Header file: TPA16x16.h
 * Created on: 2018年5月16日
 * Author: Wuxy
 * TPA 16.16C 5572 L4.7 模块 SPI通信库
 *
 * History:
 *  v13
 *    2018-05-30 调整帧预处理参数
 *    2018-05-28 增加[setTimeout(uint32_t)]，提供“非阻塞”数据更新
 ******************************************************************************/
#include "TPA16x16.h"

/*******************File Private***********************************************/
//----------Device Independent------------------------------------------------//
static const uint8_t SPI_READ = 0x00;
// Communication
static const uint8_t CMD_HEADER = 0x7F;

static const uint8_t CMD_READ_STATUS       = 0x00;
static const uint8_t CMD_READ_FRAME        = 0x01;
static const uint8_t CMD_READ_TAMB         = 0x02;
static const uint8_t CMD_READ_PIXEL_OFFSET = 0x03;
static const uint8_t CMD_READ_PIXEL_SENSIT = 0x04;
static const uint8_t CMD_READ_ALL_PARM     = 0x05;
static const uint8_t CMD_READ_PARM         = 0x06;

static const uint8_t CMD_READ              = 0x01;
static const uint8_t CMD_READ_FRAME_MODE   = 0x00;

static const uint8_t STATUS_AVAILABLE_MASK = 0x20;

//----------Device Dependent--------------------------------------------------//
// Data processing
// PROGMEM attribute dose not support 2D Array.
static const uint16_t PIXELS_OFFSET[TPA16x16::NUMBER_OF_PIXELS] PROGMEM = {
  0x7F70, 0x7F60, 0x7F83, 0x7F6D, 0x7F8A, 0x7F6E, 0x7F8A, 0x7F70, 0x7F86, 0x7F68, 0x7F73, 0x7F70, 0x7F7B, 0x7F6A, 0x7F86, 0x7F77,
  0x7F74, 0x7F67, 0x7F88, 0x7F6F, 0x7F8A, 0x7F71, 0x7F92, 0x7F70, 0x7F8B, 0x7F6A, 0x7F7C, 0x7F73, 0x7F7B, 0x7F6C, 0x7F8B, 0x7F7A,
  0x7F74, 0x7F67, 0x7F8A, 0x7F75, 0x7F90, 0x7F73, 0x7F92, 0x7F71, 0x7F8B, 0x7F6A, 0x7F7E, 0x7F7A, 0x7F81, 0x7F6E, 0x7F8F, 0x7F7F,
  0x7F7B, 0x7F6A, 0x7F90, 0x7F74, 0x7F94, 0x7F77, 0x7F91, 0x7F78, 0x7F93, 0x7F6F, 0x7F7F, 0x7F7C, 0x7F85, 0x7F71, 0x7F90, 0x7F80,
  0x7F78, 0x7F6D, 0x7F8D, 0x7F76, 0x7F94, 0x7F79, 0x7F96, 0x7F78, 0x7F91, 0x7F73, 0x7F81, 0x7F80, 0x7F86, 0x7F70, 0x7F90, 0x7F7E,
  0x7F7E, 0x7F6C, 0x7F91, 0x7F77, 0x7F99, 0x7F78, 0x7F9A, 0x7F7A, 0x7F95, 0x7F76, 0x7F83, 0x7F7E, 0x7F85, 0x7F72, 0x7F94, 0x7F84,
  0x7F7D, 0x7F71, 0x7F91, 0x7F7A, 0x7F9A, 0x7F7D, 0x7F9C, 0x7F79, 0x7F97, 0x7F74, 0x7F84, 0x7F7D, 0x7F89, 0x7F73, 0x7F94, 0x7F84,
  0x7F77, 0x7F69, 0x7F90, 0x7F76, 0x7F95, 0x7F7C, 0x7F98, 0x7F7B, 0x7F99, 0x7F75, 0x7F83, 0x7F7F, 0x7F8C, 0x7F73, 0x7F97, 0x7F85,
  0x7F7D, 0x7F9D, 0x7F85, 0x7F9F, 0x7F7F, 0x7FB4, 0x7F7B, 0x7FB2, 0x7F94, 0x7FAC, 0x7F80, 0x7F9C, 0x7F82, 0x7F9E, 0x7F7F, 0x7F92,
  0x7F7F, 0x7FA2, 0x7F86, 0x7FA0, 0x7F7E, 0x7FB6, 0x7F7D, 0x7FB2, 0x7F8F, 0x7FA9, 0x7F7E, 0x7F9D, 0x7F82, 0x7F9D, 0x7F7D, 0x7F93,
  0x7F7D, 0x7FA1, 0x7F86, 0x7FA3, 0x7F80, 0x7FB7, 0x7F7F, 0x7FAF, 0x7F91, 0x7FAD, 0x7F7F, 0x7F9C, 0x7F7F, 0x7F9F, 0x7F7F, 0x7F96,
  0x7F7C, 0x7FA3, 0x7F83, 0x7FA4, 0x7F80, 0x7FB4, 0x7F7C, 0x7FB3, 0x7F8D, 0x7FAB, 0x7F7F, 0x7F99, 0x7F80, 0x7F9A, 0x7F7F, 0x7F96,
  0x7F7F, 0x7FA0, 0x7F84, 0x7FA0, 0x7F7F, 0x7FB3, 0x7F7C, 0x7FB2, 0x7F90, 0x7FA8, 0x7F7E, 0x7F9B, 0x7F7F, 0x7F9B, 0x7F7D, 0x7F91,
  0x7F78, 0x7F9F, 0x7F7E, 0x7F9F, 0x7F7E, 0x7FB3, 0x7F79, 0x7FAF, 0x7F8E, 0x7FA9, 0x7F7C, 0x7F9B, 0x7F7B, 0x7F9B, 0x7F7D, 0x7F91,
  0x7F75, 0x7F9C, 0x7F7C, 0x7F9D, 0x7F7A, 0x7FB0, 0x7F76, 0x7FAF, 0x7F8D, 0x7FA4, 0x7F78, 0x7F9B, 0x7F7A, 0xF3FC, 0x7F7C, 0x7F8F,
  0x7F72, 0x7F96, 0x7F7D, 0x7F9D, 0x7F75, 0x7FB0, 0x7F72, 0x7FAA, 0x7F86, 0x7FA3, 0x7F73, 0x7F94, 0x7F77, 0x7F93, 0x7F76, 0x7F8F
}; // Individual Offset of Pixel
static const uint16_t PIXELS_SENSIT[TPA16x16::NUMBER_OF_PIXELS] PROGMEM = {
  0x029A, 0x02C2, 0x02F3, 0x0300, 0x0319, 0x0331, 0x033E, 0x033F, 0x035E, 0x034E, 0x035B, 0x034B, 0x0336, 0x0320, 0x0316, 0x02F1,
  0x02CA, 0x02F5, 0x0328, 0x033D, 0x0358, 0x036B, 0x037A, 0x037D, 0x0399, 0x0392, 0x0392, 0x0388, 0x037B, 0x035F, 0x034A, 0x0325,
  0x02F8, 0x0326, 0x0356, 0x036A, 0x0383, 0x03A4, 0x03AF, 0x03B6, 0x03CB, 0x03CC, 0x03CD, 0x03B9, 0x03A9, 0x038C, 0x037E, 0x0352,
  0x0319, 0x0359, 0x037E, 0x03A1, 0x03B0, 0x03B6, 0x03C9, 0x03D3, 0x03EF, 0x03F3, 0x03F7, 0x03E1, 0x03D1, 0x03BD, 0x03A5, 0x0377,
  0x0342, 0x0374, 0x03A5, 0x03C3, 0x03D8, 0x03E4, 0x03FF, 0x040A, 0x041D, 0x0418, 0x0418, 0x0408, 0x03F8, 0x03E6, 0x03CE, 0x0398,
  0x0355, 0x0393, 0x03C2, 0x03E0, 0x03F8, 0x0414, 0x041F, 0x0424, 0x0431, 0x0433, 0x0434, 0x041F, 0x041B, 0x03FF, 0x03E7, 0x03B5,
  0x0373, 0x03A8, 0x03D7, 0x03F9, 0x040D, 0x0426, 0x043A, 0x0444, 0x0447, 0x0445, 0x0449, 0x0437, 0x042B, 0x0415, 0x03FC, 0x03C7,
  0x0388, 0x03BF, 0x03E8, 0x0410, 0x0428, 0x043D, 0x044C, 0x044E, 0x0455, 0x045C, 0x045D, 0x0445, 0x043A, 0x0422, 0x0407, 0x03D5,
  0x0384, 0x03D1, 0x03EF, 0x0426, 0x043D, 0x0454, 0x045C, 0x0449, 0x045B, 0x0477, 0x0464, 0x044F, 0x0447, 0x042F, 0x0409, 0x03E5,
  0x0383, 0x03DA, 0x03F5, 0x042C, 0x0446, 0x0458, 0x046C, 0x045B, 0x046D, 0x0485, 0x0472, 0x0459, 0x0451, 0x0439, 0x0413, 0x03EB,
  0x0388, 0x03DC, 0x03F7, 0x042F, 0x0446, 0x0462, 0x0470, 0x0468, 0x0479, 0x0485, 0x0472, 0x045D, 0x0458, 0x0435, 0x041A, 0x03EB,
  0x0387, 0x03D7, 0x03FA, 0x0426, 0x0444, 0x0464, 0x0472, 0x0459, 0x0475, 0x0488, 0x046B, 0x0461, 0x0456, 0x043A, 0x0416, 0x03E6,
  0x0379, 0x03CD, 0x03ED, 0x041C, 0x0438, 0x0450, 0x045E, 0x0453, 0x0463, 0x0477, 0x045E, 0x0452, 0x0445, 0x0427, 0x03FD, 0x03DD,
  0x0376, 0x03C0, 0x03E1, 0x040B, 0x0423, 0x0444, 0x044E, 0x043F, 0x044B, 0x0466, 0x0450, 0x043A, 0x0436, 0x0418, 0x03F0, 0x03D1,
  0x0363, 0x03AD, 0x03CF, 0x03FC, 0x0413, 0x0429, 0x042E, 0x0421, 0x0432, 0x044F, 0x043E, 0x0427, 0x041F, 0x0400, 0x03E1, 0x03C6,
  0x034D, 0x0393, 0x03A4, 0x03D1, 0x03ED, 0x0400, 0x040F, 0x03FE, 0x040E, 0x0426, 0x0419, 0x0409, 0x0405, 0x03E7, 0x03C0, 0x03A2
}; // Pixel Sensitivity / 1000

static const uint16_t TAMB_SLOPE =  3303; // Tamb sensor slope PS
static const int16_t TAMB_OFFSET = -4530; // Tamb sensor offset PO

static const uint16_t SC         =  2023;        // Component Sensitivity [counts/W/m^2]
static const float EPSILON       =  0.95;        // Emissivity [%]
static const float SIGMA         =  5.670373e-8; // Stefan Boltzmann-constant [W m^-2 K^-4]

// Help Constants
static const float K_SCSE           = SC * SIGMA * EPSILON;  // SC * sigma * epsilon
static const float K_1_SCSE         = 1 / K_SCSE;            // 1 / SC / sigma / epsilon
static const float K_1_SCSE_POW_1_4 = 1 / pow(K_SCSE, 0.25); // 1 / (SC * sigma * epsilon) ^ 1/4

/*******************Class Public***********************************************/
TPA16x16::TPA16x16() : mTimeout(0), mStatus(0x00), mAmbientTemp(0) {
}

void TPA16x16::begin() {
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000UL, MSBFIRST, SPI_MODE3));
}

void TPA16x16::setTimeout(uint32_t timeout) {
  mTimeout = timeout;
}

uint8_t TPA16x16::readStatus() {
  SPI.transfer(CMD_HEADER);
  // according to default Td in TPA's document.
  delayMicroseconds(9);
  SPI.transfer(CMD_READ_STATUS);
  delayMicroseconds(9);

  mStatus = SPI.transfer(SPI_READ);
  delayMicroseconds(9);
  // consume the debugging byte.
  SPI.transfer(SPI_READ);
  delayMicroseconds(9);

  return mStatus;
}

inline bool TPA16x16::avaliable() {
  return readStatus() & STATUS_AVAILABLE_MASK;
}

bool TPA16x16::updateData() {
  uint32_t start = millis();
  while (!avaliable()) {
    if (millis() - start > mTimeout)
      // timeout.
      return false;

    // 'non-blocking' wait.
    yield();
    if (serialEventRun) serialEventRun();
  }

  SPI.transfer(CMD_HEADER);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ_FRAME);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ_FRAME_MODE);
  delayMicroseconds(9);

  // according to the document.
  delayMicroseconds(1000);

  // receive 2 Bytes (16-bit integer) from SPI and pass to 'tambRaw'.
  uint16_t tambRaw = SPI.transfer(SPI_READ) << 8;
  delayMicroseconds(9);
  tambRaw |= SPI.transfer(SPI_READ);
  delayMicroseconds(9);

  // get all pixels' data by the way.
  for (uint16_t i = 0; i < NUMBER_OF_PIXELS; i ++) {
    mBuffer[i] = SPI.transfer(SPI_READ) << 8;
    delayMicroseconds(9);
    mBuffer[i] |= SPI.transfer(SPI_READ);
    delayMicroseconds(9);
  }

  // Tamb[°C] = PS * Tamb signal / 1e6 + PO / 1e2
  mAmbientTemp = ((float)tambRaw * TAMB_SLOPE + TAMB_OFFSET * 1e4) * 1e-6;

  return true;
}

inline float TPA16x16::getAmbientTemperature() {
  return mAmbientTemp;
}

float TPA16x16::getPixel(uint8_t x, uint8_t y) {
  uint16_t index = x * NUMBER_OF_ROWS + y;

  // Normalized Pixel Signal = (Pixel Signal - Pixel Offset) / Pixel Sensitivity
  // Tobj[K] = ((Normalized Pixel Signal / ((SC / 1000)*sigma*epsilon)) + Tamb[K] ^ 4) ^ 1/4
  float b = pow(mAmbientTemp + 273, 4) * K_SCSE;

  return pow(
      (int16_t)(mBuffer[index] - pgm_read_word(PIXELS_OFFSET + index)) *
      1e6f / pgm_read_word(PIXELS_SENSIT + index) + b
      , 0.25) * K_1_SCSE_POW_1_4 - 273;
}

float* TPA16x16::getFrame(float &ambientTemp) {
  // Normalized Pixel Signal = (Pixel Signal - Pixel Offset) / Pixel Sensitivity
  // Tobj[K] = ((Normalized Pixel Signal / ((SC / 1000)*sigma*epsilon)) + Tamb[K] ^ 4) ^ 1/4

  float b = pow(mAmbientTemp + 273, 4) * K_SCSE;
  for (uint16_t i = 0; i < NUMBER_OF_PIXELS; i ++) {
    mPixels[i] = pow(
      (int16_t)(mBuffer[i] - pgm_read_word(PIXELS_OFFSET + i)) *
      1e6f / pgm_read_word(PIXELS_SENSIT + i) + b
      , 0.25) * K_1_SCSE_POW_1_4 - 273;
  }

  ambientTemp = mAmbientTemp;
  return mPixels;
}

float* TPA16x16::getPrePorcessedFrame(float &ambientTemp) {
  // this function moved the slow calculating (power 1/4) to upper computer.
  // the processing on upper computer should be:
  // Tobj[°C] = (Pixel * 1e7) ^ 1/4 - 273

  float b = pow(mAmbientTemp + 273, 4) * K_SCSE;
  for (uint16_t i = 0; i < NUMBER_OF_PIXELS; i ++) {
    mPixels[i] = ((int16_t)(mBuffer[i] - pgm_read_word(PIXELS_OFFSET + i)) *
      1e6f / pgm_read_word(PIXELS_SENSIT + i) + b) * K_1_SCSE * 1e-7f;
  }

  ambientTemp = mAmbientTemp;
  return mPixels;
}

inline uint16_t* TPA16x16::getRawFrame() {
  return mBuffer;
}

float TPA16x16::readAmbientTemperature() {
  SPI.transfer(CMD_HEADER);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ_TAMB);
  delayMicroseconds(9);

  delayMicroseconds(1200);

  uint16_t tambRaw = SPI.transfer(SPI_READ) << 8;
  delayMicroseconds(9);
  tambRaw |= SPI.transfer(SPI_READ);
  delayMicroseconds(9);

  // Tamb[°C] = PS * Tamb signal / 1e6 + PO / 1e2
  mAmbientTemp = ((float)tambRaw * TAMB_SLOPE + TAMB_OFFSET * 1e4) * 1e-6;

  return mAmbientTemp;
}

uint16_t* TPA16x16::readPixelOffset() {
  SPI.transfer(CMD_HEADER);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ_PIXEL_OFFSET);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ);
  delayMicroseconds(9);

  delayMicroseconds(6000);

  // mBuffer[] used as a temporary storage.
  for (uint16_t i = 0; i < NUMBER_OF_PIXELS; i ++) {
    mBuffer[i] = SPI.transfer(SPI_READ) << 8;
    delayMicroseconds(9);
    mBuffer[i] |= SPI.transfer(SPI_READ);
    delayMicroseconds(9);
  }

  return mBuffer;
}

uint16_t* TPA16x16::readPixelSensitivity() {
  SPI.transfer(CMD_HEADER);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ_PIXEL_SENSIT);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ);
  delayMicroseconds(9);

  delayMicroseconds(6000);

  for (uint16_t i = 0; i < NUMBER_OF_PIXELS; i ++) {
    mBuffer[i] = SPI.transfer(SPI_READ) << 8;
    delayMicroseconds(9);
    mBuffer[i] |= SPI.transfer(SPI_READ);
    delayMicroseconds(9);
  }

  return mBuffer;
}

uint16_t TPA16x16::readParameter(uint16_t index) {
  uint16_t result;

  SPI.transfer(CMD_HEADER);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ_PARM);
  delayMicroseconds(9);
  SPI.transfer(highByte(index));
  delayMicroseconds(9);
  SPI.transfer(lowByte(index));
  delayMicroseconds(9);

  // 1.2 ms, according to the document.
  delayMicroseconds(1200);

  result = SPI.transfer(SPI_READ) << 8;
  delayMicroseconds(9);
  result |= SPI.transfer(SPI_READ);
  delayMicroseconds(9);

  return result;
}

uint16_t* TPA16x16::readAllParameter() {
  SPI.transfer(CMD_HEADER);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ_ALL_PARM);
  delayMicroseconds(9);
  SPI.transfer(CMD_READ);
  delayMicroseconds(9);

  delayMicroseconds(6000);

  for (uint16_t i = 0; i < NUMBER_OF_PARMS; i ++) {
    mBuffer[i] = SPI.transfer(SPI_READ) << 8;
    delayMicroseconds(9);
    mBuffer[i] |= SPI.transfer(SPI_READ);
    delayMicroseconds(9);
  }

  return mBuffer;
}

void TPA16x16::end() {
  SPI.endTransaction();
  SPI.end();
}
