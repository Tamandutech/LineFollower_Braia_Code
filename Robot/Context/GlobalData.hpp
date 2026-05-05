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

enum Action : uint8_t { None = 0, Run, Map, CustomAction };

struct MapPoint {
  int32_t          encoderAverage;
  float            baseMotorPWM;
  float            baseVacuumPWM;
  Leds::ColorIndex colorIndex;
};

struct GlobalData {
  volatile Action          action;
  std::atomic<bool>        isReadyToRun;
  std::atomic<int32_t>     finishLineCount;
  std::vector<MapPoint>    mapData;
  std::atomic<std::size_t> markCount;
};

extern GlobalData globalData;

#endif /* CONTEXT_GLOBALDATA_HPP_ */

// Aram Khachaturian: Masquerade Suite
