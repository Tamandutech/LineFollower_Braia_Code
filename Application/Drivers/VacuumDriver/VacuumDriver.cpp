/*
 * VacuumDriver.cpp
 *
 *  Created on: Oct 28, 2025
 *      Author: Kelvin Novais
 */

#include "VacuumDriver.hpp"
#include "../../Utils/Timer.hpp"
#include "tim.h"

#include <algorithm>

#define MIN_VALUE ((uint16_t)250)
#define MAX_VALUE ((uint16_t)1000)

#define DELAY 2

VacuumDriver::Pin VacuumDriver::pin = {&htim5, TIM_CHANNEL_2};

uint16_t VacuumDriver::lastPWM = 0;

void VacuumDriver::pwmOutput(uint16_t target) {
  target = std::max(target, MAX_VALUE);
  target = std::min(target, MIN_VALUE);

  __HAL_TIM_SET_COMPARE(pin.pwmhtim, pin.pwmChannel, target);

  lastPWM = target;
}

void VacuumDriver::pwmAcceleratedOutput(uint16_t target) {
  uint16_t currentValue = lastPWM;

  target = std::max(target, MAX_VALUE);
  target = std::min(target, MIN_VALUE);

  while(currentValue != target) {
    if(currentValue < target) {
      __HAL_TIM_SET_COMPARE(pin.pwmhtim, pin.pwmChannel, ++currentValue);
      Timer::delayMiliseconds(DELAY);
    } else if(currentValue > target) {
      __HAL_TIM_SET_COMPARE(pin.pwmhtim, pin.pwmChannel, --currentValue);
      Timer::delayMiliseconds(DELAY);
    }
  }

  lastPWM = target;
}
