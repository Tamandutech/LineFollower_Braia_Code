/*
 * EncoderDriver.hpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_ENCODERS_ENCODERS_HPP_
#define DRIVERS_ENCODERS_ENCODERS_HPP_

#include <cstdint>

#include "encoders.h"

class Encoders {
public:
  typedef _Encoder Encoder;

  static int32_t getAverage();
  static int32_t getCounter(Encoder encoder);
  static void    setCounter(Encoder encoder, uint32_t value);
  static void    reset();
};

#endif /* DRIVERS_ENCODERS_ENCODERS_HPP_ */

// Tchaikovsky: Pax de Deux
