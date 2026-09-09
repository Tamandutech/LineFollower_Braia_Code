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
  static const int32_t &average;
  static const int32_t (&counter)[N_SIDES_];

  static void initialize();
  static void reset();
  static void update();
  static void setCounter(Side side, uint32_t value);

private:
  static int32_t average_;
  static int32_t counter_[N_SIDES_];
};

#endif /* DRIVERS_ENCODERS_ENCODERS_HPP_ */

// Tchaikovsky: Pax de Deux
