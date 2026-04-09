/*
 * EncoderDriver.hpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_ENCODERDRIVER_ENCODERDRIVER_HPP_
#define DRIVERS_ENCODERDRIVER_ENCODERDRIVER_HPP_

#include "encoders.h"
#include <cstdint>

class EncoderDriver {
public:
  typedef _Encoder Encoder;

  static int32_t getAverage();
  static int32_t getCounter(Encoder encoder);
  static void    setCounter(Encoder encoder, uint32_t value);
  static void    reset();
};

#endif /* DRIVERS_ENCODERDRIVER_ENCODERDRIVER_HPP_ */

// Tchaikovsky: Pax de Deux
