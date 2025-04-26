#ifndef LED_WS2812_H
#define LED_WS2812_H

#include <stdint.h>

#include "platform_functions.h"

void setLedsColor(pinhandler_t ledsPin, uint32_t* colors, uint8_t ledsCount);

#endif  // LED_WS2812_H