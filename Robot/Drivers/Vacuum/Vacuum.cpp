/*
 * Vacuum.cpp
 *
 *  Created on: Oct 28, 2025
 *      Author: Kelvin Novais
 */

#include "Vacuum.hpp"

#include <algorithm>

#define EXPOSE_VACUUM_PERIPHERAL
#include "../../Context/PeripheralsEnv.hpp"
#include "../../Context/RobotEnv.hpp"
#include "../../Utils/Timer/Timer.hpp"

Vacuum::Pin Vacuum::pin     = {PeripheralsEnv::VACUUM_TIMER,
                               PeripheralsEnv::VACUUM_CHANNEL};
uint16_t    Vacuum::lastPWM = 0;

void Vacuum::initialize() {
  static bool initialized = false;

  if(initialized) {
    // TODO error
  }

  HAL_TIM_PWM_Start(PeripheralsEnv::VACUUM_TIMER,
                    PeripheralsEnv::VACUUM_CHANNEL);

  initialized = true;
}

void Vacuum::pwmOutput(uint16_t target) {
  target = std::min(target, static_cast<uint16_t>(RobotEnv::VACUUM_MAX_PWM));

  __HAL_TIM_SET_COMPARE(pin.pwmhtim, pin.pwmChannel, target);

  lastPWM = target;
}

void Vacuum::pwmAcceleratedOutput(uint16_t target) {
  uint16_t currentValue = lastPWM;

  target = std::max(target, static_cast<uint16_t>(RobotEnv::VACUUM_MIN_PWM));
  target = std::min(target, static_cast<uint16_t>(RobotEnv::VACUUM_MAX_PWM));

  while(currentValue != target) {
    if(currentValue < target) {
      __HAL_TIM_SET_COMPARE(pin.pwmhtim, pin.pwmChannel, ++currentValue);
      Timer::delayMiliseconds(RobotEnv::VACUUM_INTERVAL_BETWEEN_INCREMENTS);
    } else if(currentValue > target) {
      __HAL_TIM_SET_COMPARE(pin.pwmhtim, pin.pwmChannel, --currentValue);
      Timer::delayMiliseconds(RobotEnv::VACUUM_INTERVAL_BETWEEN_INCREMENTS);
    }
  }

  Timer::delayMiliseconds(1500);

  lastPWM = target;
}

void Vacuum::stopAfter(uint32_t miliseconds) {
  if(lastPWM != 0) {
    Timer::delayMiliseconds(miliseconds);
    pwmOutput(0);
  }
}