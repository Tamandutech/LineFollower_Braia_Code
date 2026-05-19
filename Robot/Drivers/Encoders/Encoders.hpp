/*
 * Encoders.hpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_ENCODERS_ENCODERS_HPP_
#define DRIVERS_ENCODERS_ENCODERS_HPP_


#include "../../Context/Definitions.hpp"

class Encoders {
public:
  static void    initialize();
  static int32_t getAverage();
  static int32_t getCounter(Side side);
  static void    setCounter(Side side, uint32_t value);
  static void    reset();
};

#endif /* DRIVERS_ENCODERS_ENCODERS_HPP_ */

// Tchaikovsky: Pax de Deux
