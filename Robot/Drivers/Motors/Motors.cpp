/*
 * Motors.cpp
 *
 *  Created on: Oct 25, 2025
 *      Author: Samuel Oliveira
 *      Author: Kelvin Novais
 */

#include "Motors.hpp"

#include <algorithm>
#include <cstdint>

#define EXPOSE_MOTORS_PERIPHERAL
#include "../../Context/PeripheralsEnv.hpp"
#include "../../Context/RobotEnv.hpp"
#include "../../Services/PID/PID.hpp"
#include "../../Utils/Timer/Timer.hpp"

const Motors::Pin Motors::motorPins[N_SIDES_] = {
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
float   Motors::motorSpeed[N_SIDES_];
int16_t Motors::motorPWM[N_SIDES_];

void Motors::initialize() {
  static bool initialized = false;

  if(initialized) {
    // TODO error
  }

  HAL_TIM_PWM_Start(PeripheralsEnv::MOTOR_LEFT_TIMER,
                    PeripheralsEnv::MOTOR_LEFT_CHANNEL);
  HAL_TIM_PWM_Start(PeripheralsEnv::MOTOR_RIGHT_TIMER,
                    PeripheralsEnv::MOTOR_RIGHT_CHANNEL);

  initialized = true;
}

void Motors::pwmOutputFor(Side side, int16_t duty) {
  if(side >= N_SIDES_) {
    // TODO warning
    return;
  }

  if(duty >= 0) {
    // Defines the direction
    HAL_GPIO_WritePin(motorPins[side].dirPort, motorPins[side].dirPin,
                      GPIO_PIN_RESET);
  } else {
    // Inverts the direction
    HAL_GPIO_WritePin(motorPins[side].dirPort, motorPins[side].dirPin,
                      GPIO_PIN_SET);
    // Makes the duty positive
    duty = -duty;
  }

  // Make sure the duty is within the allowed interval
  duty            = std::min(duty, RobotEnv::MOTOR_MAX_PWM);
  motorPWM[side] = duty;
  __HAL_TIM_SET_COMPARE(motorPins[side].pwmhtim, motorPins[side].pwmChannel,
                        duty);
}

void Motors::pwmOutput(float desiredSpeed) {
  int u = PID::getPID();

  // The "u" refers to a rotational PID that corrects robot position, so we add
  // an subtract from left and right motors, respectively
  motorSpeed[Left]  = desiredSpeed + u;
  motorSpeed[Right] = desiredSpeed - u;

  pwmOutputFor(Right, motorSpeed[Right]);
  pwmOutputFor(Left, motorSpeed[Left]);
}

void Motors::stop() {
  pwmOutputFor(Left, 0);
  pwmOutputFor(Right, 0);
}

void Motors::brake() {
  pwmOutputFor(Left, -RobotEnv::MOTOR_MAX_PWM);
  pwmOutputFor(Right, -RobotEnv::MOTOR_MAX_PWM);

  Timer::delayMiliseconds(RobotEnv::MOTOR_BRAKE_TIME);

  stop();
}
