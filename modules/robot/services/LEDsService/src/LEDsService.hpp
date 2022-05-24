#ifndef LEDS_SERVICE_H
#define LEDS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "driver/gpio.h"
#include "esp32-hal.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

using namespace cpp_freertos;

#define NUM_LEDS 3
#define DATA_PIN 32
#define BRIGHTNESS 100
#define LED_TYPE WS2811
#define COLOR_ORDER RGB
#define N_COLORS 17

#define CYCLES_T0H (F_CPU * 0.0000004)     // 0.4us
#define CYCLES_RESET (F_CPU * 0.000120)      // 120us
#define CYCLES_T1H (F_CPU * 0.0000008)     // 0.8us
#define CYCLES_PERIOD (F_CPU * 0.00000125) // 1.25us per bit

enum led_color_t
{
    // Red, Green, Blue
    LED_COLOR_BLACK = 0x000000,
    LED_COLOR_RED = 0xFF0000,
    LED_COLOR_GREEN = 0x00FF00,
    LED_COLOR_BLUE = 0x0000FF,
    LED_COLOR_YELLOW = 0xFFFF00,
    LED_COLOR_CYAN = 0x00FFFF,
    LED_COLOR_MAGENTA = 0xFF00FF,
    LED_COLOR_WHITE = 0xFFFFFF,
    LED_COLOR_PURPLE = 0x7F007F,
    LED_COLOR_ORANGE = 0xFF7F00,
    LED_COLOR_BROWN = 0x7F3F00,
    LED_COLOR_LIME = 0x3FFF00,
    LED_COLOR_PINK = 0xFF007F,
    LED_COLOR_TURQUOISE = 0x00FF7F,
    LED_COLOR_VIOLET = 0x7F00FF,
};

enum led_position_t
{
    LED_POSITION_NONE = -1,
    LED_POSITION_LEFT = 0,
    LED_POSITION_RIGHT = 1,
    LED_POSITION_FRONT = 2,
};

enum led_effect_t
{
    LED_EFFECT_NONE = 0,
    LED_EFFECT_SET = 1,
    LED_EFFECT_BLINK = 2,
    LED_EFFECT_FADE = 3,
};

struct LEDColor
{
    uint8_t blue = 0;
    uint8_t red = 0;
    uint8_t green = 0;
};

struct led_command_t
{
    led_position_t led[NUM_LEDS] = {LED_POSITION_NONE};
    led_color_t color;
    led_effect_t effect;
    float brightness;
};

class LEDsService : public Thread
{
public:
    static LEDsService *getInstance(std::string name = "LEDsService", uint32_t stackDepth = 10000, UBaseType_t priority = 9)
    {
        LEDsService *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
            std::lock_guard<std::mutex> myLock(instanceMutex);
            sin = instance.load(std::memory_order_relaxed);
            if (!sin)
            {
                sin = new LEDsService(name, stackDepth, priority);
                instance.store(sin, std::memory_order_release);
            }
        }

        return sin;
    };

    esp_err_t queueCommand(led_command_t command);

    void Run() override;

private:
    static std::atomic<LEDsService *> instance;
    static std::mutex instanceMutex;

    uint8_t *colorsRGB;
    LEDColor LEDs[NUM_LEDS];

    static led_command_t ledCommand;
    static QueueHandle_t queueLedCommands;

    void led_effect_set();
    void led_effect_blink();
    void led_effect_fade();

    esp_err_t setLEDColor(uint8_t led, uint8_t red, uint8_t green, uint8_t blue);
    esp_err_t sendToLEDs();

    LEDsService(std::string name, uint32_t stackDepth, UBaseType_t priority);
};

#endif