/*
 * BLE.hpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#ifndef SERVICES_BLE_BLE_HPP_
#define SERVICES_BLE_BLE_HPP_

#include <cstdint>

class BLE {
public:
  typedef enum : uint8_t { None = 0, Run, Map, CustomAction } Action;

  static volatile Action action;

  static void start();
  static void restart();
};

#endif /* SERVICES_BLE_BLE_HPP_ */

// Boccherini: Minuet for string quintet - Op. 11, No. 5 (G 275)
