/*
 * Encoders.hpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_ENCODERS_ENCODERS_HPP_
#define DRIVERS_ENCODERS_ENCODERS_HPP_

#include <cstdint>

class Encoders {
public:
  enum Encoder : uint8_t {
    Left = 0,
    Right,

    N_ENCODERS_
  };

  static void    initialize();
  static int32_t getAverage();
  static int32_t getCounter(Encoder encoder);
  static void    setCounter(Encoder encoder, uint32_t value);
  static void    reset();
};

#endif /* DRIVERS_ENCODERS_ENCODERS_HPP_ */

// Tchaikovsky: Pax de Deux
