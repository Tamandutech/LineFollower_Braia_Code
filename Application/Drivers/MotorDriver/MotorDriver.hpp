/*
 * MotorDriver.hpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Samuel Oliveira
 *      Author: Kelvin Novais
 */

#ifndef DRIVERS_MOTORDRIVER_MOTORDRIVER_HPP_
#define DRIVERS_MOTORDRIVER_MOTORDRIVER_HPP_

#include "../../Utils/Logger/Logger.hpp"

#include "stm32g4xx_hal.h"
#include <cstdint>

class MotorDriver {
public:
  enum Motors : uint8_t {
    Left = 0,
    Right,

    _N_MOTORS
  };

  static void pwmOutput(float duty);
  static void pwmOutputFor(Motors motor, int16_t duty);
  static void stop();

private:
  typedef struct _Pin {
    // Motor direction pin
    GPIO_TypeDef *dirPort;
    uint16_t      dirPin;
    // Motor PWM pin
    TIM_HandleTypeDef *pwmhtim;
    // PWM channel
    uint32_t pwmChannel;
  } Pin;

  // Motors motor;

  const static Pin motorPins[_N_MOTORS];
  static float     motorSpeed[_N_MOTORS];
  static int16_t   motorPWM[_N_MOTORS];
};

#endif /* DRIVERS_MOTORDRIVER_MOTORDRIVER_HPP_ */

// Wagner: Der Ring des Nibelungen
