/*
 * BLE.cpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */


#include "BLE.hpp"

#define EXPOSE_BLE_PERIPHERAL
#include "../../Context/PeripheralsEnv.hpp"

#include "../../Context/GlobalData.hpp"


static uint8_t rx_buffer[32] = {0};

void BLE::initialize() {
  static bool initilized = false;

  if(initilized) {
    // TODO error
  }

  // Initialize DMA reception
  HAL_UART_Receive_DMA(PeripheralsEnv::BLE_BUS,
                       static_cast<uint8_t *>(rx_buffer), 1);

  initilized = true;
}

void BLE::start() {
  // Call DMA reception function again
  HAL_UART_Receive_DMA(PeripheralsEnv::BLE_BUS,
                       static_cast<uint8_t *>(rx_buffer), 1);
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
