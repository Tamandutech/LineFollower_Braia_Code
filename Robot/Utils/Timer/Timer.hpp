/*
 * Timer.hpp
 *
 *  Created on: Oct 29, 2025
 *      Author: Kelvin Novais
 */

#ifndef UTILS_TIMER_HPP_
#define UTILS_TIMER_HPP_

#include "tim.h"

class Timer {
public:
  enum Type : uint8_t { Miliseconds, Microseconds, Nanoseconds };

  Timer(Type type);

  uint32_t getElapsedTime();
  void     start();
  void     reset();

  // Static functions
  static uint32_t        getMiliseconds();
  static uint32_t        getNanoseconds();
  inline static uint32_t getMicroseconds() {
    return ((TIM2->CNT) / 10U); // NOLINT
  };

  static void        delayMiliseconds(uint32_t miliseconds);
  static void        delayNanoseconds(uint32_t nanoseconds);
  inline static void delayMicroseconds(uint32_t microseconds) {
    uint32_t tickStart = getMicroseconds();
    while((getMicroseconds() - tickStart) < microseconds) {
      // Do nothing
    }
  };

protected:
  Type     type;
  uint32_t tickStart;

  uint32_t getTickByType();
};

#endif /* UTILS_TIMER_HPP_ */

// Beethoven: Duet mit zwei obligaten Augengläsern, WoO 32
