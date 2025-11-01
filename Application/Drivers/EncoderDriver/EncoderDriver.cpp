/*
 * EncoderDriver.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#include "EncoderDriver.hpp"
#include "tim.h"

// TODO tmp
#include "platform_functions.h"

TIM_HandleTypeDef *EncoderDriver::encoders[_N_ENCODERS] = {
  &htim4,
  &htim3
};

int32_t EncoderDriver::getCounter(Encoder index) {
  // if (index >= _N_ENCODERS) {
  //   // TODO emit error
  //   index = Left;
  // }

  // return (int32_t)__HAL_TIM_GET_COUNTER(encoders[index]);

  if (index == Left) {
    return get_left_encoder_position();
  } else {
    return get_right_encoder_position();
  }
}

void  EncoderDriver::setCounter(Encoder index, uint32_t value) {
  // if (index >= _N_ENCODERS) {
  //   // TODO emit error
  //   index = Left;
  // }

  // __HAL_TIM_SET_COUNTER(encoders[index], value);

  if (index == Left) {
    set_left_encoder_position(value);
  } else {
    set_right_encoder_position(value);
  }
}
