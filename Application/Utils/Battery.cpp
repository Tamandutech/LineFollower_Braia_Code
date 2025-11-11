/*
 * Battery.cpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#include "Battery.hpp"
#include "adc.h"

float Battery::getBatteryVoltage() {
  // Calculates battery voltage based on ADC reading

  // Vref = 1.2V, 4095 is the maximum value of ADC
  float vref = 1.21F * 4095.0F / *rawReferenceVoltage;

  // 5.6875 is the voltage divider constant
  float vbat = (*rawBatteryVoltage * vref / 4095.0F) * 6.015F;

  return vbat;
}
