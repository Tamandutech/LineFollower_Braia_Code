/*
 * EncoderDriver.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "tim.h"

#include "../../Utils/Logger/Logger.hpp"
#include "Encoders.hpp"

static Logger *logger = new Logger("EncoderDriver", true, Logger::Level::All);

/* 
 * Here we declare static variables to make them "private" to this file, but
 * still visible to the C callback function
 */
static uint32_t           encoderValues[Encoders::_N_ENCODERS]   = {0};
static uint16_t           encoderOverflow[Encoders::_N_ENCODERS] = {0};
static TIM_HandleTypeDef *encoders[Encoders::_N_ENCODERS] = {&htim4, &htim3};

int32_t Encoders::getAverage() {
  return ((getCounter(Left) + getCounter(Right)) / 2);
}

int32_t Encoders::getCounter(Encoder index) {
  if(index >= _N_ENCODERS) {
    logger->error("Invalid encoder");
    index = Left;
  }

  encoderValues[index] = ((uint32_t)encoderOverflow[index] << 16) +
                         (uint16_t)__HAL_TIM_GET_COUNTER(encoders[index]);

  return static_cast<int32_t>(encoderValues[index]);
}

void Encoders::setCounter(Encoder index, uint32_t value) {
  if(index >= _N_ENCODERS) {
    logger->error("Invalid encoder");
    index = Left;
  }

  encoderValues[index]   = value;
  encoderOverflow[index] = value >> 16;

  __HAL_TIM_SET_COUNTER(encoders[index], (uint16_t)(value & 0xFFFF));
}

void Encoders::reset() {
  for(uint8_t index = 0; index < _N_ENCODERS; index++) {
    __HAL_TIM_SET_COUNTER(encoders[index], 0);
    encoderValues[index]   = 0;
    encoderOverflow[index] = 0;
  }
}

// The callback must be at C scope
extern "C" {
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  /*
   * This function receives a pointer to a timer (the "htim");
   *
   * We run a loop comparing the the received pointer and the pointers stored on
   * "encoders" array, in order to find out which is the index of the array
   */
  int index = 0;
  for(index = 0; index < Encoders::_N_ENCODERS; index++) {
    if(encoders[index] == htim) break;
  }

  // Checks whether it is overflow (counting up) or underflow (counting down)
  if(__HAL_TIM_IS_TIM_COUNTING_DOWN(htim)) {
    // Underflow
    encoderOverflow[index]--;
  } else {
    // Overflow
    encoderOverflow[index]++;
  }

  encoderValues[index] = ((uint32_t)encoderOverflow[index] << 16) +
                         (uint16_t)__HAL_TIM_GET_COUNTER(htim);
}
}
