/*
 * EncoderDriver.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "EncoderDriver.hpp"
#include "../../Utils/Logger.hpp"
#include "encoders.h"

#include <cstdint>

static Logger *logger = new Logger(
    "EncoderDriver", true,
    static_cast<Logger::Level>(Logger::Level::Info | Logger::Level::Debug));

int32_t EncoderDriver::getCounter(Encoder index) {
  if(index >= _N_ENCODERS) {
    logger->error("Invalid encoder");
    index = Left;
  }

  encoderValues[index] = ((uint32_t)encoderOverflow[index] << 16) +
                         (uint16_t)__HAL_TIM_GET_COUNTER(encoders[index]);

  return static_cast<int32_t>(encoderValues[index]);
}

void EncoderDriver::setCounter(Encoder index, uint32_t value) {
  if(index >= _N_ENCODERS) {
    logger->error("Invalid encoder");
    index = Left;
  }

  encoderValues[index]   = value;
  encoderOverflow[index] = value >> 16;

  __HAL_TIM_SET_COUNTER(encoders[index], (uint16_t)(value & 0xFFFF));
}

void EncoderDriver::reset() {
  for(uint8_t index = 0; index < _N_ENCODERS; index++) {
    __HAL_TIM_SET_COUNTER(encoders[index], 0);
    encoderValues[index]   = 0;
    encoderOverflow[index] = 0;
  }
}