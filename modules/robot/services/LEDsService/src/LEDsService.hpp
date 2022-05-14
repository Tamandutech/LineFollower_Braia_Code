#ifndef LEDS_SERVICE_H
#define LEDS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "driver/gpio.h"
#include "esp32-hal.h"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

using namespace cpp_freertos;

#define NUM_LEDS 3
#define DATA_PIN 32
#define BRIGHTNESS 100
#define LED_TYPE WS2811
#define COLOR_ORDER RGB
#define N_COLORS 17

#define CYCLES_800_T0H  (F_CPU / 2500001) // 0.4us
#define CYCLES_800_T1H  (F_CPU / 1250001) // 0.8us
#define CYCLES_800      (F_CPU /  800001) // 1.25us per bit


struct LEDColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};



class LEDsService : public Thread
{
public:
    LEDsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    RobotStatus *status;
    uint8_t *colorsRGB;
    LEDColor LEDs[NUM_LEDS];

    esp_err_t setLEDColor(uint8_t led, uint8_t red, uint8_t green, uint8_t blue);
    esp_err_t sendToLEDs();
};

#endif