/*
 * PID.hpp
 *
 *  Created on: Nov 12, 2025
 *      Author: Kelvin Novais
 */

#ifndef SERVICES_PID_PID_HPP_
#define SERVICES_PID_PID_HPP_

class PID {
public:
  static float getPID();

private:
  static float lastError;
};


#endif /* SERVICES_PID_PID_HPP_ */
