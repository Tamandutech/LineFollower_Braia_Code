/*
 * Leds.hpp
 *
 *  Created on: Oct 27, 2025
 *      Author: Kelvin Novais
 *      Author: Samuel Oliveira
 */

/*******************************************************************************
 * @file  Leds.hpp
 * @brief  Driver for addressable WS2812B LEDs, using TIM PWM and DMA.
 * @note  This driver was developed for the Celeris Core S1
 * @note  and depends on the configuration of timer TIM1 in CubeMX.
 * @note  Make sure to use the correct .ioc file for the project configuration.
 * This implementation uses a high-frequency PWM (6.4 MHz) to generate the
 * precise timing of 800 kHz for the data bus.
 ******************************************************************************/

#ifndef DRIVERS_LEDS_LEDS_HPP_
#define DRIVERS_LEDS_LEDS_HPP_

#include <cstdint>

/*
 * The timer is set to 6.4 MHz, 8 times the 800kHz data frequency of the WS2812
 */
#define PWM_CYCLES_PER_BIT 8

/*
 * 8 bits for G, 8 for R, 8 for B
 */
#define LED_BITS 24

/*
 * The reset pulse needs at least 50 us
 * Our PWM cycle is 156.25 ns (1/6.4 MHz)
 * 50,000ns / 156.25ns = 320 cycles. We're going to set a higher number for
 * safety
 */
#define RESET_CYCLES 330

#define WAVE_AMPLITUDE Leds::maxColorValue
#define WAVE_PERIOD    (WAVE_AMPLITUDE * 2)
#define TRIANGULAR_WAVE(x) \
  ((uint8_t)abs(((x += 2) % WAVE_PERIOD) - WAVE_AMPLITUDE))

/*
 * Driver configuration:
 * Define a mnemonic for each LED and, by consequence, the number of LEDs your
 * strip will have.
 * The memory buffer will be statically allocated for this size.
 */
enum Led : uint8_t {
  MainBoardLed = 0,
  CenterLed,
  RightLed,
  LeftLed,

  N_LEDS_
};

enum ColorIndex : uint8_t {
  // Rotatable colors
  FirstRotatableColor = 0,
  Red                 = FirstRotatableColor,
  Blue,
  Green,
  Magenta,
  Indigo,
  Orange,
  Cyan,
  Yellow,
  LastRotatableColor,

  // Not rotatable colors
  White,
  Black,

  N_COLOR_INDEXES_
};

class Leds {
public:
  // Defining a color type
  struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  struct WavingColor {
    int16_t r;
    int16_t g;
    int16_t b;
  };

  struct PredefinedColors {
    RGB         rgb;
    const char *name;
  };

  static const PredefinedColors color[N_COLOR_INDEXES_];
  static const uint8_t          maxColorValue;

  static void setColorForAll(RGB rgb);
  static void setColorForAll(ColorIndex color);

  static void setColorFor(Led led, RGB rgb);
  static void setColorFor(Led led, ColorIndex color);

private:
  static RGB ledsColors[N_LEDS_];
  /*
   * DMA Static buffer for DMA:
   * Main buffer. Declared as 'static' to avoid stack overflow.
   * The size is calculated based on the maximum LEDs and the reset pulse.
   * We use uint32_t to match the size of the timer register (CCR).
   */
  static uint32_t pwmBuffer[RESET_CYCLES + RESET_CYCLES +
                            N_LEDS_ * LED_BITS * PWM_CYCLES_PER_BIT];

  static void outputColors();
};

#endif /* DRIVERS_LEDS_LEDS_HPP_ */

// Satie: Gymnopedies 1 & 3 (Orchestration: Debussy)
