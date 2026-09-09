/*
 * Mapper.hpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#ifndef SERVICES_MAPPER_MAPPER_HPP_
#define SERVICES_MAPPER_MAPPER_HPP_

#include <vector>

#include "../../Utils/Logger/Logger.hpp"

class Mapper {
public:
  static void map();

private:
  struct MappingData {
    // FLOATS
    /*
     * 32 bits each float
     */
    float x;
    float y;
    float omega;


    // WORD 1
    /*
     * Can hold up to:       2³⁰ - 1 = 1,073,741,823 µs
     * which is              1,073.7 s
     * which is              17.9 min
     */
    uint32_t timestamp   : 30;
    uint32_t isLeftMark  : 1;
    uint32_t isRightMark : 1;


    // WORD 2
    /*
     * Must hold up to TRACK_MAX_PULSES: 2²² - 1 = 4,194,303 pulses
     */
    uint32_t leftEncoder : 22;
    /*
     * Must hold up to LastRotatableColor
     */
    uint32_t colorIndex  : 3;
    uint32_t padding1    : 7;


    // WORD 3
    /*
     * Must hold up to TRACK_MAX_PULSES: 2²² - 1 = 4,194,303 pulses
     */
    uint32_t rightEncoder : 22;
    uint32_t padding2     : 10;

  } __attribute__((packed));

  static Logger *logger;

  static void logMap(std::vector<MappingData> &mapping);
};

#endif /* SERVICES_MAPPER_MAPPER_HPP_ */

// Niccolò Paganini: Caprice No. 24
