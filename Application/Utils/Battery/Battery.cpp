/*
 * Battery.cpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#include "Battery.hpp"
#include "adc.h"

/*
 * TSince the ADC buffers are in a messy order, we are going to expose a pointer
 * to the desired variable, matching the correct ADC address.
 */
uint32_t *const Battery::rawBatteryVoltage = &adc2_buffer[8];
uint32_t *const Battery::rawReferenceVoltage = &adc1_buffer[8];

float Battery::getBatteryVoltage() {
  // Calculates battery voltage based on ADC reading

  // Vref = 1.2V, 4095 is the maximum value of ADC
  float vref = 1.21F * 4095.0F / *rawReferenceVoltage;

  /*
    TODO fix constant value
    
    BATTERY 1:
      REAL      7.91 V
      MESUARING 8.38 V

    BATTERY 2:
      REAL      8.33 V
      MEASURING 8.82 V

    BATTERY 3:
      REAL      8.09 V
      MEASURING 8.57 V
  */

  // 5.6875 is the voltage divider constant
  float vbat = (*rawBatteryVoltage * vref / 4095.0F) * 5.6875F;

  return vbat;
}
