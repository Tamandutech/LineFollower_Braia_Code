/*
 * Battery.hpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#ifndef UTILS_BATTERY_HPP_
#define UTILS_BATTERY_HPP_

#include <cstdint>

class Battery {
public:
  static float getBatteryVoltage();

private:
  static uint32_t *const rawBatteryVoltage;
  static uint32_t *const rawReferenceVoltage;
};

#endif /* UTILS_BATTERY_HPP_ */
