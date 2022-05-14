#include "LEDsService.hpp"

LEDsService::LEDsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->status = robot->getStatus();

#ifndef ESP32_QEMU
    FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(8, 2000);

    ws2812fx = new WS2812FX();
    segments = ws2812fx->getSegments();

    ws2812fx->init(NUM_LEDS, leds, false); // type was configured before

    ws2812fx->setBrightness(BRIGHTNESS);
    ws2812fx->setMode(0 /*segid*/, FX_MODE_STATIC);
    ws2812fx->setMode(1 /*segid*/, FX_MODE_STATIC);
    ws2812fx->setMode(2 /*segid*/, FX_MODE_STATIC);
    segments[0].colors[0] = CRGB::Black;
    status->ColorLed0->setData(CRGB::Black);
    status->ColorLed1->setData(CRGB::Black);
    status->ColorLed2->setData(CRGB::Black);
    ws2812fx->service();
#endif
}

void LEDsService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
#ifndef ESP32_QEMU
        ws2812fx->service();
#endif
        if(status->robotIsMapping->getData() || status->encreading->getData())
        {
            if(status->robotState->getData() == CAR_IN_LINE){
                 status->ColorLed0->setData(CRGB::Green);
            }
            else if(status->robotState->getData() == CAR_IN_CURVE){
                 status->ColorLed0->setData(CRGB::White);
            }
        }
        segments[0].colors[0] = status->ColorLed0->getData();
        segments[1].colors[0] = status->ColorLed1->getData();
        segments[2].colors[0] = status->ColorLed2->getData();
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
    }
}