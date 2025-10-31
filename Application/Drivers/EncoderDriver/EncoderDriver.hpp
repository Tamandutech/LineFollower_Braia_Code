/*
 * EncoderDriver.hpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_ENCODERDRIVER_ENCODERDRIVER_HPP_
#define DRIVERS_ENCODERDRIVER_ENCODERDRIVER_HPP_

#include "stm32g4xx_hal.h"

class EncoderDriver {
public:
  enum Encoder : uint8_t {
    Left = 0,
    Right,

    _N_ENCODERS
  };

  // TODO avoid overflow  
  static int32_t getCounter(Encoder encoder);
  static void setCounter(Encoder encoder, int32_t value);

private:
  static TIM_HandleTypeDef *encoders[_N_ENCODERS];
};

#endif /* DRIVERS_ENCODERDRIVER_ENCODERDRIVER_HPP_ */

/*
  Tchaikovsky: Pax de Deux
*/