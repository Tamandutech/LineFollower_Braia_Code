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


class Mapper {
public:
  static void map();

private:
  static uint8_t qtdLeftMark;
  static uint8_t qtdRightMark;
  static bool    readRightBefore;
  static bool    readLeftBefore;
  static bool    readIntersecBefore;
  static bool    firstTimeRight;
  static uint8_t n_marks;
  static Logger *logger;

  static void readLateral();
};

#endif /* SERVICES_MAPPER_MAPPER_HPP_ */
