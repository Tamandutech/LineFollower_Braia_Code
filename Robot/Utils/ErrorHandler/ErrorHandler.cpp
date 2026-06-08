/*
 * ErrorHandler.cpp
 *
 *  Created on: Jun 3, 2026
 *      Author: Kelvin Novais
 */

#include "ErrorHandler.h"

#include "../../Drivers/Leds/Leds.hpp"
#include "../../Drivers/Motors/Motors.hpp"
#include "../../Drivers/Vacuum/Vacuum.hpp"
#include "../../Utils/Logger/Logger.hpp"

void emergency_stop() {
  Leds::setColorForAll(Red);
  Motors::stop();
  Vacuum::pwmOutput(0);
}

void on_hard_fault() {
  Logger::log("Hard fault");
  Leds::setColorFor(CenterLed, Yellow);
}

void on_memory_management_fault() {
  Logger::log("Memory management fault");
  Leds::setColorFor(CenterLed, Indigo);
}

void on_bus_fault() {
  Logger::log("Bus fault");
  Leds::setColorFor(CenterLed, White);
}

void on_usage_fault() {
  Logger::log("Usage fault");
  Leds::setColorFor(CenterLed, Magenta);
}