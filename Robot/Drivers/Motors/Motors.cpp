/*
 * Motors.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Samuel Oliveira
 *      Author: Kelvin Novais
 */

#include "Motors.hpp"

#include <algorithm>

#define EXPOSE_MOTORS_PERIPHERAL
#include "../../Context/PeripheralsEnv.hpp"
#include "../../Context/RobotEnv.hpp"
#include "../../Utils/Logger/Logger.hpp"
#include "../../Utils/Timer/Timer.hpp"

const Motors::Pin Motors::motorPins_[N_SIDES_] = {
    // Left
    [Left] = {PeripheralsEnv::MOTOR_LEFT_DIRECTION_PORT,
              PeripheralsEnv::MOTOR_LEFT_DIRECTION_PIN,
              PeripheralsEnv::MOTOR_LEFT_TIMER,
              PeripheralsEnv::MOTOR_LEFT_CHANNEL },

    // Right
    [Right] = {PeripheralsEnv::MOTOR_RIGHT_DIRECTION_PORT,
              PeripheralsEnv::MOTOR_RIGHT_DIRECTION_PIN,
              PeripheralsEnv::MOTOR_RIGHT_TIMER,
              PeripheralsEnv::MOTOR_RIGHT_CHANNEL}
};
float   Motors::motorSpeed_[N_SIDES_];
int16_t Motors::motorPWM_[N_SIDES_];

static Logger *logger = new Logger("Motors", true, Logger::Level::All);

void Motors::initialize() {
  static bool initialized = false;

  if(initialized) {
    logger->error("Motors already initialized, unexpected behaviour.");
    return;
  }

  HAL_TIM_PWM_Start(PeripheralsEnv::MOTOR_LEFT_TIMER,
                    PeripheralsEnv::MOTOR_LEFT_CHANNEL);
  HAL_TIM_PWM_Start(PeripheralsEnv::MOTOR_RIGHT_TIMER,
                    PeripheralsEnv::MOTOR_RIGHT_CHANNEL);

  initialized = true;
}

void Motors::pwmOutputFor(Side side, int16_t duty) {
  if(side >= N_SIDES_) {
    logger->error("Invalid motor side");
    return;
  }

  if(duty >= 0) {
    // Defines the direction
    HAL_GPIO_WritePin(motorPins_[side].dirPort, motorPins_[side].dirPin,
                      GPIO_PIN_RESET);
  } else {
    // Inverts the direction
    HAL_GPIO_WritePin(motorPins_[side].dirPort, motorPins_[side].dirPin,
                      GPIO_PIN_SET);
    // Makes the duty positive
    duty = -duty;
  }

  // Make sure the duty is within the allowed interval
  duty            = std::min(duty, (int16_t)MOTOR_MAX_PWM);
  motorPWM_[side] = duty;
  __HAL_TIM_SET_COMPARE(motorPins_[side].pwmhtim, motorPins_[side].pwmChannel,
                        duty);
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
