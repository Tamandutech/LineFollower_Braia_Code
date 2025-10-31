/*
 * EncoderDriver.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "EncoderDriver.hpp"
#include "tim.h"

TIM_HandleTypeDef *EncoderDriver::encoders[_N_ENCODERS] = {
  &htim4,
  &htim3
};

int32_t EncoderDriver::getCounter(Encoder index) {
  if (index >= _N_ENCODERS) {
    // TODO emit error
    index = Left;
  }

  return (int32_t)__HAL_TIM_GET_COUNTER(encoders[index]);
}

void  EncoderDriver::setCounter(Encoder index, int32_t value) {
  if (index >= _N_ENCODERS) {
    // TODO emit error
    index = Left;
  }

  __HAL_TIM_SET_COUNTER(encoders[index], value);
}
