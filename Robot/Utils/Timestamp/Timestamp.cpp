/*
 * Timestamp.cpp
 *
 *  Created on: Nov 2, 2025
 *      Author: Kelvin Novais
 */

#include "../../Utils/Timestamp/Timestamp.hpp"

#include <cstdint>

Timestamp::Timestamp() : Timer(Miliseconds) {
  Timer::start();
};

void Timestamp::start() {
  Timer::start();
}

void Timestamp::reset() {
  Timer::reset();
}

void Timestamp::getTimestamp(uint8_t *minutes,
                             uint8_t *seconds,
                             uint32_t *milliseconds) {
  uint32_t elapsedTime = getElapsedTime();

  *minutes = elapsedTime / 60000;
  *seconds = (elapsedTime % 60000) / 1000;
  *milliseconds = elapsedTime % 1000;
}