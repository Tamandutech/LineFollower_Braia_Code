/*
 * VacuumDriver.cpp
 *
 *  Created on: Oct 28, 2025
 *      Author: Kelvin Novais
 */

#include "VacuumDriver.hpp"
#include "../../Context/RobotEnv.hpp"
#include "../../Utils/Timer/Timer.hpp"
#include "tim.h"

#include <algorithm>

VacuumDriver::Pin VacuumDriver::pin = {&htim5, TIM_CHANNEL_2};

uint16_t VacuumDriver::lastPWM = 0;

void VacuumDriver::pwmOutput(uint16_t target) {
  target = std::min(target, static_cast<uint16_t>(RobotEnv::MAX_MOTOR_PWM));

  __HAL_TIM_SET_COMPARE(pin.pwmhtim, pin.pwmChannel, target);

  lastPWM = target;
}

void VacuumDriver::pwmAcceleratedOutput(uint16_t target) {
  uint16_t currentValue = lastPWM;

  target = std::max(target, static_cast<uint16_t>(RobotEnv::BASE_VACUUM_PWM));
  target = std::min(target, static_cast<uint16_t>(RobotEnv::MAX_MOTOR_PWM));

  while(currentValue != target) {
    if(currentValue < target) {
      __HAL_TIM_SET_COMPARE(pin.pwmhtim, pin.pwmChannel, ++currentValue);
      Timer::delayMiliseconds(RobotEnv::VACUUM_INTERVAL_BETWEEN_INCREMENTS);
    } else if(currentValue > target) {
      __HAL_TIM_SET_COMPARE(pin.pwmhtim, pin.pwmChannel, --currentValue);
      Timer::delayMiliseconds(RobotEnv::VACUUM_INTERVAL_BETWEEN_INCREMENTS);
    }
  }

  Timer::delayMiliseconds(500);

  lastPWM = target;
}

void VacuumDriver::stopAfter(uint32_t miliseconds) {
  if(lastPWM != 0) {
    Timer::delayMiliseconds(miliseconds);
    pwmOutput(0);
  }
}