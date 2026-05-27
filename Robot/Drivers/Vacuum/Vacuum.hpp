/*
 * Vacuum.hpp
 *
 *  Created on: Oct 28, 2025
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_VACUUM_VACUUM_HPP_
#define DRIVERS_VACUUM_VACUUM_HPP_

#include "stm32g4xx_hal.h"

class Vacuum {
public:
  static void initialize();
  static void pwmOutput(uint16_t duty);
  static void pwmAcceleratedOutput(uint16_t duty);
  static void stopAfter(uint32_t miliseconds);

private:
  struct Pin {
    // PWM timer
    TIM_HandleTypeDef *pwmhtim;
    // PWM channel
    uint32_t pwmChannel;
  };

  static Pin      pin;
};

#endif /* DRIVERS_VACUUM_VACUUM_HPP_ */
