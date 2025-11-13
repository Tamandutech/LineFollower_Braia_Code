/*
 * VacuumDriver.hpp
 *
 *  Created on: Oct 28, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_VACUUMDRIVER_VACUUMDRIVER_HPP_
#define DRIVERS_VACUUMDRIVER_VACUUMDRIVER_HPP_

#include "stm32g4xx_hal.h"

#include <cstdint>

class VacuumDriver {
public:
  static void pwmOutput(uint16_t duty);
  static void pwmAcceleratedOutput(uint16_t duty);
  static void stopAfter(uint32_t miliseconds);

private:
  typedef struct _Pin {
    // Motor PWM pin
    TIM_HandleTypeDef *pwmhtim;
    // PWM channel
    uint32_t pwmChannel;
  } Pin;

  static Pin      pin;
  static uint16_t lastPWM;
};

#endif /* DRIVERS_VACUUMDRIVER_VACUUMDRIVER_HPP_ */
