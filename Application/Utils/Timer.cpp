/*
 * Timer.cpp
 *
 *  Created on: Oct 29, 2025
 *      Author: Kelvin Novais
 */

#include "Timer.hpp"
#include "stm32g4xx_hal.h"

Timer::Timer(Type newType) {
  type = newType;

  switch (type) {
  case Microseconds:
    tickStart = getMicroseconds();
    break;

  case Nanoseconds:
    tickStart = getNanoseconds();
    break;

  default:
  case Miliseconds:
    tickStart = getMiliseconds();
    break;
  }
}

uint32_t Timer::getElapsedTime() {
  uint32_t now = 0;

  switch (type) {
  case Microseconds:
    now = getMicroseconds();
    break;

  case Nanoseconds:
    now = getNanoseconds();
    break;

  default:
  case Miliseconds:
    now = getMiliseconds();
    break;
  }

  return (now - tickStart);
}

void Timer::reset() {
  switch (type) {
  case Microseconds:
    tickStart = getMicroseconds();
    break;

  case Nanoseconds:
    tickStart = getNanoseconds();
    break;

  default:
  case Miliseconds:
    tickStart = getMiliseconds();
    break;
  }
}

uint32_t Timer::getTickByType() {
  switch (type) {
  case Microseconds:
    return getMicroseconds();
    break;

  case Nanoseconds:
    return getNanoseconds();
    break;

  default:
  case Miliseconds:
    return getMiliseconds();
    break;
  }
}

uint32_t Timer::getMiliseconds() {
  return HAL_GetTick();
}

uint32_t Timer::getMicroseconds() {
  return (uint32_t)((TIM2->CNT) / (uint32_t)10);
}

uint32_t Timer::getNanoseconds() {
  return (uint32_t)((TIM2->CNT) * 100U);
}

void Timer::delayMiliseconds(uint32_t miliseconds) {
  uint32_t tickStart = getMiliseconds();
  while ((getMiliseconds() - tickStart) < miliseconds) {
    // No Operation
    __NOP();
  }
}

// TODO test with __NOP()
void Timer::delayMicroseconds(uint32_t microseconds) {
  uint32_t tickStart = getMicroseconds();
  while ((getMicroseconds() - tickStart) < microseconds) {
    // Do nothing
  }
}

// TODO test with __NOP()
void Timer::delayNanoseconds(uint32_t nanoseconds) {
  uint32_t tickStart = getNanoseconds();
  while ((getNanoseconds() - tickStart) < nanoseconds) {
    // Do nothing
  }
}
