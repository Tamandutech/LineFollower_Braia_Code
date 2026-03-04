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

#include "../Drivers/LedDriver/LedDriver.hpp"

typedef struct _MapPoint {
  int32_t                    encoderAverage;
  float                      baseMotorPWM;
  float                      baseVacuumPWM;
  const LedDriver::RgbColor &color;
} MapPoint;


typedef struct _GlobalData {
  std::atomic<bool>        isReadyToRun;
  std::atomic<int32_t>     finishLineCount;
  std::vector<MapPoint>    mapData;
  std::atomic<std::size_t> markCount;
} GlobalData;

extern GlobalData globalData;

#endif /* CONTEXT_GLOBALDATA_HPP_ */

// Aram Khachaturian: Masquerade Suite
