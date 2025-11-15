/*
 * Timer.hpp
 *
 *  Created on: Oct 29, 2025
 *      Author: Kelvin Novais
 */

#ifndef UTILS_TIMER_HPP_
#define UTILS_TIMER_HPP_

#include <cstdint>

// TODO need to test all functions
class Timer {
public:
  enum Type : uint8_t {
    Miliseconds,
    Microseconds,
    Nanoseconds
  };

  Timer(Type type);

  uint32_t getElapsedTime();
  void start();
  void reset();

  // Static functions
  static uint32_t getMiliseconds();
  static uint32_t getMicroseconds();
  static uint32_t getNanoseconds();

  static void delayMiliseconds(uint32_t miliseconds);
  static void delayMicroseconds(uint32_t microseconds);
  static void delayNanoseconds(uint32_t nanoseconds);

protected:
  Type type;
  uint32_t tickStart;

  uint32_t getTickByType();
};

#endif /* UTILS_TIMER_HPP_ */
