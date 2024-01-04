#ifndef LEDS_SERVICE_H
#define LEDS_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"
#include "LedStatus.hpp"


#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "led_strip_encoder.h"
#include <sys/cdefs.h>

#include "esp_log.h"

using namespace cpp_freertos;


#define WS2812_T0H_NS (350)
#define WS2812_T0L_NS (1000)
#define WS2812_T1H_NS (1000)
#define WS2812_T1L_NS (350)
#define WS2812_RESET_US (280)

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      GPIO_NUM_32
#define NUM_LEDS 3

enum led_position_t
{
    LED_POSITION_LEFT = 0,
    LED_POSITION_RIGHT = 1,
    LED_POSITION_FRONT = 2,
};

enum led_effect_t
{
    LED_EFFECT_SET = 0,
    LED_EFFECT_BLINK = 1,
    LED_EFFECT_FADE = 2,
};

struct LEDColor
{
    uint8_t blue = 0;
    uint8_t red = 0;
    uint8_t green = 0;
};

struct led_command_t
{
    led_position_t led;
    led_color_t color;
    led_effect_t effect;
    float brightness;
};

class LEDsService : public Thread, public Singleton<LEDsService>
{
public:
    LEDsService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void LedComandSend(led_position_t led,  led_color_t color, float brightness, led_effect_t effect = LED_EFFECT_SET);

    void Run() override;

private:
    rmt_channel_handle_t led_chan = NULL;
    rmt_encoder_handle_t led_encoder = NULL;
    rmt_transmit_config_t tx_config;

    LEDColor LEDs[NUM_LEDS];

    static led_command_t ledCommand;
    uint8_t led_strip_pixels[NUM_LEDS * 3];
    static QueueHandle_t queueLedCommands;
    esp_err_t queueCommand(led_command_t command);

    void led_effect_set();
    void led_color_set(led_color_t color, float brightness, led_position_t pos);
    void led_RGB_get(led_color_t color, uint8_t * R, uint8_t * G, uint8_t * B);
    void led_strip_init();
    void led_strip_refresh();
};

#endif