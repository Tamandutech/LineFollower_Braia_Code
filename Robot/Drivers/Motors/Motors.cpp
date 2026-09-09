/*
 * Motors.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Samuel Oliveira
 *      Author: Kelvin Novais
 */

/******************************************************************************/
// INCLUDES
#include "Motors.hpp"

#include <algorithm>

#include "tim.h"

#include "../../Context/RobotEnv.hpp"
#include "../../Utils/Logger/Logger.hpp"
#include "../../Utils/Timer/Timer.hpp"

/******************************************************************************/
// PERIPHERALS
const Motors::Pin Motors::pins_[N_SIDES_] = {
    // Left
    [Left] = {motor2dir_GPIO_Port, motor2dir_Pin, &htim8, TIM_CHANNEL_1},

    // Right
    [Right] = {motor1dir_GPIO_Port, motor1dir_Pin, &htim8, TIM_CHANNEL_3}
};

/******************************************************************************/
// VARIABLES
float   Motors::motorSpeed_[N_SIDES_];
int16_t Motors::motorPWM_[N_SIDES_];

static Logger *logger = new Logger("Motors", true, Logger::Level::All);

void Motors::initialize() {
  static bool initialized = false;

  if(initialized) {
    logger->error("Motors already initialized, unexpected behaviour.");
    return;
  }

  HAL_TIM_PWM_Start(pins_[Left].pwmhtim, pins_[Left].pwmChannel);
  HAL_TIM_PWM_Start(pins_[Right].pwmhtim, pins_[Right].pwmChannel);

  initialized = true;
}

void Motors::pwmOutputFor(Side side, int16_t duty) {
  if(side >= N_SIDES_) {
    logger->error("Invalid motor side");
    return;
  }

  if(duty >= 0) {
    // Defines the direction
    HAL_GPIO_WritePin(pins_[side].dirPort, pins_[side].dirPin, GPIO_PIN_RESET);
  } else {
    // Inverts the direction
    HAL_GPIO_WritePin(pins_[side].dirPort, pins_[side].dirPin, GPIO_PIN_SET);
    // Makes the duty positive
    duty = -duty;
  }

  // Make sure the duty is within the allowed interval
  duty            = std::min(duty, (int16_t)MOTOR_MAX_PWM);
  motorPWM_[side] = duty;
  __HAL_TIM_SET_COMPARE(pins_[side].pwmhtim, pins_[side].pwmChannel, duty);
}

void Motors::stop() {
  pwmOutputFor(Left, 0);
  pwmOutputFor(Right, 0);
}

void Motors::brake() {
  pwmOutputFor(Left, -MOTOR_MAX_PWM);
  pwmOutputFor(Right, -MOTOR_MAX_PWM);

  Timer::delayMiliseconds(MOTOR_BRAKE_TIME);

  stop();
}
