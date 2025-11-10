/*
 * BLEListener.hpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#ifndef SERVICES_BLELISTENER_BLELISTENER_HPP_
#define SERVICES_BLELISTENER_BLELISTENER_HPP_

class BLEListener {
public:
  static volatile bool run;

  static void start();
  static void restart();
};

#endif /* SERVICES_BLELISTENER_BLELISTENER_HPP_ */
