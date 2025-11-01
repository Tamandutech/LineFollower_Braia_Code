/*
 * LedDriver.cpp
 *
 *  Created on: Oct 27, 2025
 *      Author: Kelvin Novais
 *      Author: Samuel Oliveira
 */

#include "LedDriver.hpp"

#include "tim.h"

/*
 * CCR register values ​​to generate logic levels on channel N (inverted)
 */
// For channel N to go HIGH, the main channel must have 0% of duty cycle
#define PWM_HIGH 0
// For channel N to go LOW, the main channel must have 100% of duty cycle
// (ARR=4, CCR=5)
#define PWM_LOW 5

const LedDriver::PredefinedColors LedDriver::Colors = {
  .red = {128, 0, 0},
  .green = {0, 128, 0},
  .blue = {0, 0, 128},
  .magenta = {128, 0, 128},
  .white = {128, 128, 128},
  .yellow = {128, 128, 0},
  .cyan = {9, 62, 9},
  .black = {0, 0, 0}
};

uint32_t LedDriver::pwmBuffer[RESET_CYCLES + RESET_CYCLES
                              + _N_LEDS * LED_BITS * PWM_CYCLES_PER_BIT] = {0};

// FIXME this is just a workaround. We possibly need to rewrite this function
void LedDriver::setColorForAll(RgbColor newColor) {
  // Pointer to the start of our DMA buffer
  uint32_t *buffer_ptr = pwmBuffer;

  // Add RESET pulse at the beginning
  for (int i = 0; i < RESET_CYCLES; i++) {
    *buffer_ptr++ = PWM_LOW;
  }

  // 1. Loop through each LED
  for (uint16_t i = 0; i < Leds::_N_LEDS; i++) {
    // Garante que os valores sejam pares pois por algum motivo valores ímpares causam problemas
    uint8_t r = newColor.r % 2 ? newColor.r - 1 : newColor.r;
    uint8_t g = newColor.g % 2 ? newColor.g - 1 : newColor.g;
    uint8_t b = newColor.b % 2 ? newColor.b - 1 : newColor.b;

    // GRB format is the standard for most WS2812 chips
    uint32_t color = (g << 16) | (r << 8) | b;

    // 2. Loop through the 24 color bits of each LED (G7..G0, R7..R0, B7..B0)
    for (int j = 23; j >= 0; j--) {
      if ((color >> j) & 1) {
        // Bit pattern '1': HIGH for ~780ns, LOW for ~470ns
        // For the N channel, this means:
        // 5 cycles LOW and 3 cycles HIGH, on the main channel.
        *buffer_ptr++ = PWM_HIGH;
        *buffer_ptr++ = PWM_HIGH;
        *buffer_ptr++ = PWM_HIGH;
        *buffer_ptr++ = PWM_HIGH;
        *buffer_ptr++ = PWM_HIGH;
        *buffer_ptr++ = PWM_LOW;
        *buffer_ptr++ = PWM_LOW;
        *buffer_ptr++ = PWM_LOW;
      } else {
        // Bit pattern '0': HIGH for ~470ns, LOW for ~780ns
        // For the N channel, this means:
        // 3 cycles LOW and 5 HIGH, on the main channel.
        *buffer_ptr++ = PWM_HIGH;
        *buffer_ptr++ = PWM_HIGH;
        *buffer_ptr++ = PWM_HIGH;
        *buffer_ptr++ = PWM_LOW;
        *buffer_ptr++ = PWM_LOW;
        *buffer_ptr++ = PWM_LOW;
        *buffer_ptr++ = PWM_LOW;
        *buffer_ptr++ = PWM_LOW;
      }
    }
  }

  // 3. Fill the end of the buffer with the RESET pulse
  for (int i = 0; i < RESET_CYCLES; i++) {
    *buffer_ptr++ = PWM_LOW;
  }

  // 4. Send data via DMA
  // The total size of data to be sent is the initial reset, the LEDs, and the
  // final reset.
  uint32_t data_len = Leds::_N_LEDS * LED_BITS * PWM_CYCLES_PER_BIT;
  uint32_t total_buffer_size = RESET_CYCLES + data_len + RESET_CYCLES;

  // Starts DMA transfer
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, pwmBuffer, total_buffer_size);
}
