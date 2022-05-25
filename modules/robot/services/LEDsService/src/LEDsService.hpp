#ifndef LEDS_SERVICE_H
#define LEDS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "driver/gpio.h"
#include "esp32-hal.h"
#include "driver/rmt.h"
#include <sys/cdefs.h>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

using namespace cpp_freertos;

#define NUM_LEDS 3

#define WS2812_T0H_NS (350)
#define WS2812_T0L_NS (1000)
#define WS2812_T1H_NS (1000)
#define WS2812_T1L_NS (350)
#define WS2812_RESET_US (280)

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

typedef struct led_strip_s led_strip_t;

typedef rmt_channel_t led_strip_dev_t;

/**
 * @brief Declare of LED Strip Type
 *
 */
struct led_strip_s
{
    /**
     * @brief Set RGB for a specific pixel
     *
     * @param strip: LED strip
     * @param index: index of pixel to set
     * @param red: red part of color
     * @param green: green part of color
     * @param blue: blue part of color
     *
     * @return
     *      - ESP_OK: Set RGB for a specific pixel successfully
     *      - ESP_ERR_INVALID_ARG: Set RGB for a specific pixel failed because of invalid parameters
     *      - ESP_FAIL: Set RGB for a specific pixel failed because other error occurred
     */
    esp_err_t (*set_pixel)(led_strip_t *strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue);

    /**
     * @brief Refresh memory colors to LEDs
     *
     * @param strip: LED strip
     * @param timeout_ms: timeout value for refreshing task
     *
     * @return
     *      - ESP_OK: Refresh successfully
     *      - ESP_ERR_TIMEOUT: Refresh failed because of timeout
     *      - ESP_FAIL: Refresh failed because some other error occurred
     *
     * @note:
     *      After updating the LED colors in the memory, a following invocation of this API is needed to flush colors to strip.
     */
    esp_err_t (*refresh)(led_strip_t *strip, uint32_t timeout_ms);

    /**
     * @brief Clear LED strip (turn off all LEDs)
     *
     * @param strip: LED strip
     * @param timeout_ms: timeout value for clearing task
     *
     * @return
     *      - ESP_OK: Clear LEDs successfully
     *      - ESP_ERR_TIMEOUT: Clear LEDs failed because of timeout
     *      - ESP_FAIL: Clear LEDs failed because some other error occurred
     */
    esp_err_t (*clear)(led_strip_t *strip, uint32_t timeout_ms);

    /**
     * @brief Free LED strip resources
     *
     * @param strip: LED strip
     *
     * @return
     *      - ESP_OK: Free resources successfully
     *      - ESP_FAIL: Free resources failed because error occurred
     */
    esp_err_t (*del)(led_strip_t *strip);
};

/**
 * @brief LED Strip Configuration Type
 *
 */
typedef struct
{
    uint32_t max_leds;   /*!< Maximum LEDs in a single strip */
    led_strip_dev_t dev; /*!< LED strip device (e.g. RMT channel, PWM channel, etc) */
} led_strip_config_t;

typedef struct
{
    led_strip_t parent;
    rmt_channel_t rmt_channel;
    uint32_t strip_len;
    uint8_t buffer[0];
} ws2812_t;

class LEDsService : public Thread<LEDsService>
{
public:

    LEDsService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    esp_err_t queueCommand(led_command_t command);

    void Run() override;

private:
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(GPIO_NUM_32, RMT_CHANNEL_0);
    led_strip_config_t strip_config;
    led_strip_t *strip;

    LEDColor LEDs[NUM_LEDS];

    static led_command_t ledCommand;
    static QueueHandle_t queueLedCommands;

    void led_effect_set();
    
    // Driver para LED WS2812(B)
    led_strip_t *led_strip_new_rmt_ws2812(const led_strip_config_t *config);

    static uint32_t ws2812_t0h_ticks;
    static uint32_t ws2812_t1h_ticks;
    static uint32_t ws2812_t0l_ticks;
    static uint32_t ws2812_t1l_ticks;

    static esp_err_t ws2812_del(led_strip_t *strip);
    static esp_err_t ws2812_clear(led_strip_t *strip, uint32_t timeout_ms);
    static esp_err_t ws2812_refresh(led_strip_t *strip, uint32_t timeout_ms);
    static esp_err_t ws2812_set_pixel(led_strip_t *strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue);
    static void IRAM_ATTR ws2812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
                                             size_t wanted_num, size_t *translated_size, size_t *item_num);
};

#endif