/*
 * Vacuum.cpp
 *
 *  Created on: Oct 28, 2025
 *      Author: Kelvin Novais
 */

#include "Vacuum.hpp"

#include <algorithm>

#include "tim.h"

#include "../../Context/RobotEnv.hpp"
#include "../../Drivers/Leds/Leds.hpp"
#include "../../Utils/Logger/Logger.hpp"
#include "../../Utils/Timer/Timer.hpp"

Vacuum::Pin Vacuum::pin_ = {&htim5, TIM_CHANNEL_2};

static Logger *logger = new Logger("Vacuum", true, Logger::Level::All);

void Vacuum::initialize() {
  static bool initialized = false;

  if(initialized) {
    logger->error("Vacuum already initialized, unexpected behaviour.");
    return;
  }

  HAL_TIM_PWM_Start(pin_.pwmhtim, pin_.pwmChannel);

  initialized = true;
}

void Vacuum::pwmOutput(uint16_t target) {
  target = std::min(target, static_cast<uint16_t>(VACUUM_MAX_PWM));

  __HAL_TIM_SET_COMPARE(pin_.pwmhtim, pin_.pwmChannel, target);
}

void Vacuum::pwmAcceleratedOutput(uint16_t target) {
  Leds::RGB color = {0};
  float     p     = 0;

  Leds::setColorForAll(Black);

  target = std::max(target, static_cast<uint16_t>(VACUUM_MIN_PWM));
  target = std::min(target, static_cast<uint16_t>(VACUUM_MAX_PWM));

  for(uint16_t i = 1; i <= target; i++) {
    // Scale and set color proportionally to the target value
    p       = (float)i / target;
    color.g = Leds::maxColorValue * p;                 // increase
    color.r = color.b = Leds::maxColorValue * (1 - p); // decrease
    Leds::setColorForAll(color);

    __HAL_TIM_SET_COMPARE(pin_.pwmhtim, pin_.pwmChannel, i);
    Timer::delayMiliseconds(VACUUM_INTERVAL_BETWEEN_INCREMENTS);
  }

  Timer::delayMiliseconds(1500);
}

void Vacuum::stopAfter(uint32_t miliseconds) {
  Timer::delayMiliseconds(miliseconds);
  pwmOutput(0);
}