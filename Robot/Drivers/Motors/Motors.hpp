/*
 * MotorDriver.hpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Samuel Oliveira
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_MOTORS_MOTORS_HPP_
#define DRIVERS_MOTORS_MOTORS_HPP_

#include "stm32g4xx_hal.h"
#include <cstdint>

class Motors {
public:
  enum Motor : uint8_t {
    Left = 0,
    Right,

    _N_MOTORS
  };

  static void pwmOutput(float duty);
  static void pwmOutputFor(Motor motor, int16_t duty);
  static void stop();

private:
  struct Pin {
    // Motor direction pin
    GPIO_TypeDef *dirPort;
    uint16_t      dirPin;
    // Motor PWM pin
    TIM_HandleTypeDef *pwmhtim;
    // PWM channel
    uint32_t pwmChannel;
  };

  const static Pin motorPins[_N_MOTORS];
  static float     motorSpeed[_N_MOTORS];
  static int16_t   motorPWM[_N_MOTORS];
};

#endif /* DRIVERS_MOTORS_MOTORS_HPP_ */

// Wagner: Der Ring des Nibelungen
