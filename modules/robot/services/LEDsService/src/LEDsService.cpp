#include "LEDsService.hpp"

QueueHandle_t LEDsService::queueLedCommands;
led_command_t LEDsService::ledCommand;

LEDsService::LEDsService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    ESP_LOGD(GetName().c_str(), "Constructor Start");

    queueLedCommands = xQueueCreate(10, sizeof(ledCommand));

    ESP_LOGD(GetName().c_str(), "GPIO: %d", RMT_LED_STRIP_GPIO_NUM);
    led_strip_init();
    memset(led_strip_pixels, 0, sizeof(led_strip_pixels));

    ESP_LOGD(GetName().c_str(), "Constructor END");
}

void LEDsService::Run()
{
    ESP_LOGD(GetName().c_str(), "Run");

    for (;;)
    {
        vTaskDelay(0);
        xQueueReceive(queueLedCommands, &ledCommand, portMAX_DELAY);


        switch (ledCommand.effect)
        {
        case LED_EFFECT_SET:
            led_effect_set();
            break;

        case LED_EFFECT_BLINK:
            // led_effect_blink();
            break;

        case LED_EFFECT_FADE:
            // led_effect_fade();
            break;

        default:
            break;
        }
    }
}

void LEDsService::LedComandSend(led_position_t led,  led_color_t color, float brightness, led_effect_t effect)
{
    led_command_t command = {
        .led = led,
        .color = color,
        .effect = effect,
        .brightness = brightness,
    };
    queueCommand(command);
}

esp_err_t LEDsService::queueCommand(led_command_t command)
{
    ESP_LOGD(GetName().c_str(), "queueCommand: command.effect = %d", command.effect);
    return xQueueSend(queueLedCommands, &command, portMAX_DELAY);
}

void LEDsService::led_effect_set()
{
    ESP_LOGD(GetName().c_str(), "led_effect_set");
    led_color_set(ledCommand.color, ledCommand.brightness, ledCommand.led);
    led_strip_refresh();
}

void LEDsService::led_color_set(led_color_t color, float brightness, led_position_t pos)
{
    // Lê os valores RGB da cor desejada
    uint8_t R, G, B;
    led_RGB_get(color, &R, &G, &B);
    R = brightness * R;
    G = brightness * G;
    B = brightness * B;

    // Armazena no LED Strip esses valores no padrão GBR
    led_strip_pixels[pos * 3 + 0] = G;
    led_strip_pixels[pos * 3 + 1] = B;
    led_strip_pixels[pos * 3 + 2] = R;

    ESP_LOGD(GetName().c_str(), "led_effect_set: ledCommand.led = %d, R = %d, G = %d, B = %d", pos, R, G, B);
}

void LEDsService::led_RGB_get(led_color_t color, uint8_t * R, uint8_t * G, uint8_t * B)
{
    *R = ((uint8_t *)(&ledCommand.color))[0];
    *G = ((uint8_t *)(&ledCommand.color))[1];
    *B = ((uint8_t *)(&ledCommand.color))[2];
}

void LEDsService::led_strip_init()
{
    ESP_LOGI(GetName().c_str(), "Create RMT TX channel");
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(GetName().c_str(), "Install led strip encoder");

    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(GetName().c_str(), "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    ESP_LOGI(GetName().c_str(), "Start LED rainbow chase");
    tx_config = {
        .loop_count = 0, // no transfer loop
    };
}

void LEDsService::led_strip_refresh()
{
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
}