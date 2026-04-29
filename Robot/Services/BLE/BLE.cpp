/*
 * BLEListener.cpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#include "stm32g4xx_hal.h"
#include "usart.h"
#include "BLE.hpp"

#define BLE_BUS huart1

static uint8_t               rx_buffer[32]       = {0};
volatile BLEListener::Action BLEListener::action = BLEListener::None;

void BLEListener::start() {
  // Init DMA reception
  HAL_UART_Receive_DMA(&BLE_BUS, static_cast<uint8_t *>(rx_buffer), 1);
}

void BLEListener::restart() {
  // Restart DMA reception
  start();
}

static void updateAction() {
  if(rx_buffer[0] == '1') {
    // If the buffer contains '1', it should do nothing
    BLEListener::action = BLEListener::None;
  } else if(rx_buffer[0] == '2') {
    // If contains '2', it should run
    BLEListener::action = BLEListener::Run;
  } else if(rx_buffer[0] == '3') {
    BLEListener::action = BLEListener::Map;
  } else if(rx_buffer[0] == '4') {
    BLEListener::action = BLEListener::CustomAction;
  }
}

extern "C" {
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if(huart->Instance == USART1) { // NOLINT
    // A C-compatible function to interpret the received character
    updateAction();

    // Resets DMA reception
    BLEListener::restart();
  }
}
}
