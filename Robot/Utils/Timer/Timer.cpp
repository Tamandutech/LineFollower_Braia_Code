/*
 * Timer.cpp
 *
 *  Created on: Oct 29, 2025
 *      Author: Kelvin Novais
 */

#include "../../Utils/Timer/Timer.hpp"

#include "stm32g4xx_hal.h"

Timer::Timer(Type newType) {
  type      = newType;
  tickStart = 0;
}

uint32_t Timer::getTickByType() {
  switch(type) {
  case Miliseconds: return getMiliseconds(); break;

  case Microseconds: return getMicroseconds(); break;

  case Nanoseconds: return getNanoseconds(); break;

  default: return getMiliseconds(); break;
  }
}

uint32_t Timer::getElapsedTime() {
  uint32_t now     = getTickByType();
  uint32_t elapsed = now - tickStart;

  return elapsed;
}

void Timer::start() { tickStart = getTickByType(); }

void Timer::reset() { tickStart = getTickByType(); }

uint32_t Timer::getMiliseconds() { return HAL_GetTick(); }

uint32_t Timer::getMicroseconds() {
  return ((TIM2->CNT) / 10U); // NOLINT
}

uint32_t Timer::getNanoseconds() {
  return ((TIM2->CNT) * 100U); // NOLINT
}

void Timer::delayMiliseconds(uint32_t miliseconds) {
  uint32_t tickStart = getMiliseconds();
  while((getMiliseconds() - tickStart) < miliseconds) {
    /*
     * No Operation:
     * Cosumes 1 extra cycle per iteration (1 / clock frequency)
     */
    __NOP();
  }
}

void Timer::delayMicroseconds(uint32_t microseconds) {
  uint32_t tickStart = getMicroseconds();
  while((getMicroseconds() - tickStart) < microseconds) {
    // Do nothing
  }
}

/*
 * @note This function can only perform a delay > 1 CPU clock
 * @note A busy-wait loop is unreliable, so this function might not be precise;
 * a better approach would be writing an Assembly function.
 */
void Timer::delayNanoseconds(uint32_t nanoseconds) {
  uint32_t tickStart = getNanoseconds();
  while((getNanoseconds() - tickStart) < nanoseconds) {
    // Do nothing
  }
}
