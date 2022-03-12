#ifndef LEDS_SERVICE_H
#define LEDS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "FX.h"

#include "driver/gpio.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

#define NUM_LEDS 3
#define DATA_PIN 32
#define BRIGHTNESS 100
#define LED_TYPE WS2811
#define COLOR_ORDER RGB
#define N_COLORS 17

static const CRGB colors[N_COLORS] = {
    CRGB::Red,
    CRGB::Green,
    CRGB::Blue,
    CRGB::White,
    CRGB::AliceBlue,
    CRGB::ForestGreen,
    CRGB::Lavender,
    CRGB::MistyRose,
    CRGB::DarkOrchid,
    CRGB::DarkOrange,
    CRGB::Black,
    CRGB::Teal,
    CRGB::Violet,
    CRGB::Lime,
    CRGB::Chartreuse,
    CRGB::BlueViolet,
    CRGB::Aqua};

class LEDsService : public Thread
{
public:
    LEDsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    RobotStatus *status;

    CRGB leds[NUM_LEDS];

    WS2812FX *ws2812fx;
    WS2812FX::Segment *segments;
};

#endif