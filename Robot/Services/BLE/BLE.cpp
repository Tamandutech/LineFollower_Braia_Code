/*
 * BLE.cpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#include "stm32g4xx_hal.h"
#include "usart.h"
#include "BLE.hpp"

#define BLE_BUS huart1

static uint8_t               rx_buffer[32]       = {0};
volatile BLE::Action BLE::action = BLE::None;

void BLE::start() {
  // Init DMA reception
  HAL_UART_Receive_DMA(&BLE_BUS, static_cast<uint8_t *>(rx_buffer), 1);
}

void BLE::restart() {
  // Restart DMA reception
  start();
}

static void updateAction() {
  if(rx_buffer[0] == '1') {
    // If the buffer contains '1', it should do nothing
    globalData.action = Action::None;
  } else if(rx_buffer[0] == '2') {
    // If contains '2', it should run
    globalData.action = Action::Run;
  } else if(rx_buffer[0] == '3') {
    globalData.action = Action::Map;
  } else if(rx_buffer[0] == '4') {
    globalData.action = Action::CustomAction;
  }
}

extern "C" {
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if(huart->Instance == USART1) { // NOLINT
    // A C-compatible function to interpret the received character
    updateAction();

    // Resets DMA reception
    BLE::restart();
  }
}
}
