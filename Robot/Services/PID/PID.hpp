/*
 * PID.hpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#ifndef SERVICES_PID_PID_HPP_
#define SERVICES_PID_PID_HPP_

#include <cstdint>

class PID {
public:
  static float evaluate(const int16_t irSensorError);

private:
  static float lastError;
};


#endif /* SERVICES_PID_PID_HPP_ */
