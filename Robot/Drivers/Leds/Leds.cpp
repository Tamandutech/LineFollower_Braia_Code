/*
 * Leds.cpp
 *
 *  Created on: Oct 27, 2025
 *      Author: Kelvin Novais
 *      Author: Samuel Oliveira
 */

/******************************************************************************/
// INCLUDES
#include "Leds.hpp"

#include <algorithm>
#include <cstdint>

#include "tim.h"

#include "../../Utils/Logger/Logger.hpp"

/******************************************************************************/
// PERIPHERALS
#define LEDS_TIMER   htim1
#define LEDS_CHANNEL TIM_CHANNEL_1


/******************************************************************************/
// DEFINES
/*
 * CCR register values ​​to generate logic levels on channel N (inverted)
 */
// For channel N to go HIGH, the main channel must have 0% of duty cycle
#define PWM_HIGH 0
// For channel N to go LOW, the main channel must have 100% of duty cycle
// (ARR=4, CCR=5)
#define PWM_LOW  5


/******************************************************************************/
// VARIABLES
static Logger *logger = new Logger("Leds", true, Logger::Level::All);

const Leds::PredefinedColors Leds::color[N_COLOR_INDEXES_] = {
    [Red]                = {{128, 0, 0},     "red"           },
    [Blue]               = {{0, 0, 128},     "blue"          },
    [Green]              = {{0, 128, 0},     "green"         },
    [Magenta]            = {{128, 0, 128},   "magenta"       },
    [Indigo]             = {{64, 0, 128},    "indigo"        },
    [Orange]             = {{128, 24, 0},    "orange"        },
    [Cyan]               = {{0, 128, 128},   "cyan"          },
    [Yellow]             = {{128, 64, 0},    "yellow"        },
    [LastRotatableColor] = {{0, 0, 0},       "last_rotatable"},
    [White]              = {{128, 128, 128}, "white"         },
    [Black]              = {{0, 0, 0},       "black"         }
};

const uint8_t Leds::maxColorValue                                      = 128;
Leds::RGB     Leds::ledsColors[N_LEDS_]                                = {{0}};
uint32_t      Leds::pwmBuffer[RESET_CYCLES + RESET_CYCLES +
                         N_LEDS_ * LED_BITS * PWM_CYCLES_PER_BIT] = {0};

void Leds::outputColors() {
  // Pointer to the start of our DMA buffer
  uint32_t *buffer_ptr = static_cast<uint32_t *>(pwmBuffer);

  // Add RESET pulse at the beginning
  for(int i = 0; i < RESET_CYCLES; i++) {
    *buffer_ptr++ = PWM_LOW;
  }

  // 1. Loop through each LED
  for(uint16_t i = 0; i < N_LEDS_; i++) {
    // Garante que os valores sejam pares pois por algum motivo valores ímpares
    // causam problemas
    uint8_t r = ledsColors[i].r % 2 ? ledsColors[i].r - 1 : ledsColors[i].r;
    uint8_t g = ledsColors[i].g % 2 ? ledsColors[i].g - 1 : ledsColors[i].g;
    uint8_t b = ledsColors[i].b % 2 ? ledsColors[i].b - 1 : ledsColors[i].b;


    // GRB format is the standard for most WS2812 chips
    uint32_t newColor = (g << 16) | (r << 8) | b;

    // 2. Loop through the 24 color bits of each LED (G7..G0, R7..R0, B7..B0)
    for(int j = 23; j >= 0; j--) {
      if((newColor >> j) & 1) {
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
  for(int i = 0; i < RESET_CYCLES; i++) {
    *buffer_ptr++ = PWM_LOW;
  }

  // 4. Send data via DMA
  // The total size of data to be sent is the initial reset, the LEDs, and the
  // final reset.
  uint32_t data_len          = N_LEDS_ * LED_BITS * PWM_CYCLES_PER_BIT;
  uint32_t total_buffer_size = RESET_CYCLES + data_len + RESET_CYCLES;

  // Starts DMA transfer
  HAL_TIMEx_PWMN_Start(&LEDS_TIMER, LEDS_CHANNEL);
  HAL_TIM_PWM_Start_DMA(&LEDS_TIMER, LEDS_CHANNEL,
                        static_cast<uint32_t *>(pwmBuffer), total_buffer_size);
}

void Leds::setColorForAll(RGB rgb) {
  rgb.r = std::min(rgb.r, maxColorValue);
  rgb.g = std::min(rgb.g, maxColorValue);
  rgb.b = std::min(rgb.b, maxColorValue);

  for(uint8_t i = 0; i < N_LEDS_; i++) {
    ledsColors[i] = rgb;
  }

  outputColors();
}

void Leds::setColorForAll(ColorIndex index) {
  if(index >= N_COLOR_INDEXES_) {
    logger->error("Invalid ColorIndex: %d", index);
    return;
  }

  for(uint8_t i = 0; i < N_LEDS_; i++) {
    ledsColors[i] = color[index].rgb;
  }

  outputColors();
}

void Leds::setColorFor(Led led, RGB rgb) {
  if(led >= N_LEDS_) {
    logger->error("Invalid Led: %d", led);
    return;
  }

  rgb.r = std::min(rgb.r, maxColorValue);
  rgb.g = std::min(rgb.g, maxColorValue);
  rgb.b = std::min(rgb.b, maxColorValue);

  ledsColors[led] = rgb;

  outputColors();
}

void Leds::setColorFor(Led led, ColorIndex colorIndex) {
  if(colorIndex >= N_COLOR_INDEXES_ || led >= N_LEDS_) {
    logger->error("Invalid Led (%d) or ColorIndex (%d)", led, colorIndex);
    return;
  }

  ledsColors[led] = color[colorIndex].rgb;

  outputColors();
}