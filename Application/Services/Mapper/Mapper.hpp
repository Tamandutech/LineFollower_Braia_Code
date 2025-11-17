/*
 * Mapper.hpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#ifndef SERVICES_MAPPER_MAPPER_HPP_
#define SERVICES_MAPPER_MAPPER_HPP_

#include <cstdint>

#include "../../Drivers/QTRSensorDriver/QTRSensorDriver.hpp"
#include "../../Utils/Logger/Logger.hpp"


class Mapper : private QTRSensorDriver {
public:
  // Maps the path to GlobalData
  static void map();
  static void logMap();

private:
  static uint8_t qtdLeftMark;
  static uint8_t qtdRightMark;
  static bool    readRightBefore;
  static bool    readLeftBefore;
  static bool    readIntersecBefore;
  static bool    firstTimeRight;
  static Logger *logger;

  static void  readLateral();
  static float calculatePWM();
};

#endif /* SERVICES_MAPPER_MAPPER_HPP_ */

// Niccolò Paganini: Caprice No. 24
