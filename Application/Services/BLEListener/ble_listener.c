/*
 * ble_listening.c
 *
 *  Created on: Nov 10, 2025
 *      Author: Kelvin Novais
 */

#include "stm32g4xx_hal.h"
#include "usart.h"
#include "ble_listener.h"

#if 0

#define BLE_BUS huart1

uint8_t          rx_buffer[32] = {0};
volatile uint8_t run           = 0;

void start_ble_listening(void) {
  // Init DMA reception
  HAL_UART_Receive_DMA(&BLE_BUS, rx_buffer, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if(huart->Instance == USART1) {
    // If the buffer contains "1", "run" must be 0,
    // if contains "2", run must be 1
    if(rx_buffer[0] == '1') {
      run = 0;
    } else if(rx_buffer[0] == '2') {
      run = 1;
    }

    // Resets DMA reception
    start_ble_listening();
  }
}

#endif
