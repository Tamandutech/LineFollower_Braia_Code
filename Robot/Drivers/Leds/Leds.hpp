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

class Leds {
public:
  // Defining a color type
  struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  enum ColorIndex : uint8_t {
    // Rotatable colors
    First = 0,
    Red   = First,
    Blue,
    Green,
    Magenta,
    Indigo,
    Orange,
    White,
    Cyan,
    Yellow,
    LastRotatable = Yellow,

    // Not rotatable colors
    Black,

    N_COLORS_
  };

  struct PredefinedColors {
    RGB         rgb;
    const char *name;
  };

  static const PredefinedColors color[N_COLORS_];

  /*
   * Driver configuration:
   * Define a mnemonic for each LED and, by consequence, the number of LEDs your
   * strip will have.
   * The memory buffer will be statically allocated for this size.
   */
  enum Led : uint8_t {
    MainBoard = 0,
    Center,
    Right,
    Left,

    N_LEDS_
  };

  static void setColorForAll(RGB rgb);
  static void setColorForAll(ColorIndex name);
  static void setColorFor(RGB rgb, Led led);

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
