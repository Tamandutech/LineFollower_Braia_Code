#include "LEDsService.hpp"

LEDsService::LEDsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->status = robot->getStatus();

    FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(8, 2000);

    ws2812fx = new WS2812FX();
    segments = ws2812fx->getSegments();

    ws2812fx->init(NUM_LEDS, leds, false); // type was configured before

    ws2812fx->setBrightness(BRIGHTNESS);
    ws2812fx->setMode(0 /*segid*/, FX_MODE_STATIC);
    segments[0].colors[0] = CRGB::Black;
    ws2812fx->service();
}

void LEDsService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        //ws2812fx->service();

        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
    }
}