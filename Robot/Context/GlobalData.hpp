/*
 * GlobalData.hpp
 *
 *  Created on: Nov 15, 2025
 *      Author: Kelvin Novais
 */

#ifndef CONTEXT_GLOBALDATA_HPP_
#define CONTEXT_GLOBALDATA_HPP_

#include <atomic>
#include <cstdint>
#include <vector>

#include "../Drivers/Leds/Leds.hpp"

enum class Action : uint8_t { None = 1, Run, Map };

struct MapPoint {
  int32_t    encoderAverage;
  float      baseMotorPWM;
  float      baseVacuumPWM;
  ColorIndex colorIndex;
};

struct GlobalData {
  volatile Action          action;
  std::vector<MapPoint>    map;
  std::atomic<std::size_t> markCount;
};

extern GlobalData globalData;

#endif /* CONTEXT_GLOBALDATA_HPP_ */

// Aram Khachaturian: Masquerade Suite
