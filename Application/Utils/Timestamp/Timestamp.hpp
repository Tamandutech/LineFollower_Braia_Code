/*
 * Timestamp.hpp
 *
 *  Created on: Nov 2, 2025
 *      Author: Kelvin Novais
 */

#ifndef UTILS_TIMESTAMP_HPP_
#define UTILS_TIMESTAMP_HPP_

#include "../Timer/Timer.hpp"

#include <cstdint>

class Timestamp : private Timer {
public:
  Timestamp();

  void start();
  void reset();

  void getTimestamp(uint8_t *minutes, uint8_t *seconds, uint32_t *milliseconds);
};

#endif /* UTILS_TIMESTAMP_HPP_ */

// Dvořák: 9. Sinfonie
