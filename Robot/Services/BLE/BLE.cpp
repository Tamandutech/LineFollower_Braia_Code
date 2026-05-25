/*
 * BLE.cpp
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#include <cstring>

#include "stm32g4xx_hal.h"

#include "BLE.hpp"
#include "BLEPin.h"

#define EXPOSE_BLE_PERIPHERAL
#include "../../Context/PeripheralsEnv.hpp"

#include "../../Context/GlobalData.hpp"


static uint8_t rx_buffer[32]  = {0};
static char    lastCharacter_ = ' ';

const char &BLE::lastCharacter = lastCharacter_;

void BLE::setup() {
  const uint32_t timeout     = 500; // ms
  const char    *start       = "AT\r\n";
  const char    *setName     = "AT+NAME"
                               "TT_BRAIA"
                               "\r\n";
  const char    *setPin      = "AT+PIN" BLE_PIN "\r\n";
  const char    *setBaudRate = "AT+BAUD"
                               "9"
                               "\r\n";

  HAL_Delay(5000);
  HAL_UART_Transmit(PeripheralsEnv::BLE_BUS, (uint8_t *)start, strlen(start),
                    timeout);

  // (I) Set name
  HAL_Delay(1000);

  HAL_UART_Transmit(PeripheralsEnv::BLE_BUS, (uint8_t *)setName,
                    strlen(setName), timeout);

  // (II) Set pin
  HAL_Delay(1000);
  HAL_UART_Transmit(PeripheralsEnv::BLE_BUS, (uint8_t *)setPin, strlen(setPin),
                    timeout);

  // (III) Set baud rate
  HAL_Delay(1000);
  HAL_UART_Transmit(PeripheralsEnv::BLE_BUS, (uint8_t *)setBaudRate,
                    strlen(setBaudRate), timeout);

  HAL_Delay(1000);
}

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
  }

  if(rx_buffer[0] != '\n' && rx_buffer[0] != '\r' && rx_buffer[0] != '\0')
    lastCharacter_ = rx_buffer[0];
}

void BLE::resetLastCharacter() { lastCharacter_ = ' '; }

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
