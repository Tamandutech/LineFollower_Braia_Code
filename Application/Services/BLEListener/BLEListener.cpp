/*
 * BLEListener.cpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#include "BLEListener.hpp"

#include "stm32g4xx_hal.h"
#include "usart.h"

#define BLE_BUS huart1

static uint8_t rx_buffer[32]    = {0};
volatile bool  BLEListener::run = 0;

void BLEListener::start() {
  // Init DMA reception
  HAL_UART_Receive_DMA(&BLE_BUS, rx_buffer, 1);
}

void BLEListener::restart() {
  // Restart DMA reception
  start();
}

extern "C" {
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if(huart->Instance == USART1) {
    // If the buffer contains "1", "run" must be 0,
    // if contains "2", run must be 1
    if(rx_buffer[0] == '1') {
      BLEListener::run = 0;
    } else if(rx_buffer[0] == '2') {
      BLEListener::run = 1;
    }

    // Resets DMA reception
    BLEListener::restart();
  }
}
}