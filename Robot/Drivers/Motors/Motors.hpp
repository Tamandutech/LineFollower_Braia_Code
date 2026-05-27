/*
 * Motors.hpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Samuel Oliveira
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_MOTORS_MOTORS_HPP_
#define DRIVERS_MOTORS_MOTORS_HPP_

#include "stm32g4xx_hal.h"

#include "../../Context/Definitions.hpp"

class Motors {
public:
  static void initialize();
  static void pwmOutputFor(Side side, int16_t duty);
  static void stop();
  static void brake();

private:
  struct Pin {
    // Direction port/pin
    GPIO_TypeDef *dirPort;
    uint16_t      dirPin;
    // PWM timer
    TIM_HandleTypeDef *pwmhtim;
    // PWM channel
    uint32_t pwmChannel;
  };

  const static Pin motorPins_[N_SIDES_];
  static float     motorSpeed_[N_SIDES_];
  static int16_t   motorPWM_[N_SIDES_];
};

#endif /* DRIVERS_MOTORS_MOTORS_HPP_ */

// Wagner: Der Ring des Nibelungen
