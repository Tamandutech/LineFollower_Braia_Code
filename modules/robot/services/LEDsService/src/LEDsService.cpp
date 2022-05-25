#include "LEDsService.hpp"

QueueHandle_t LEDsService::queueLedCommands;
led_command_t LEDsService::ledCommand;

LEDsService::LEDsService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    uint32_t cyclesoffset = 0;

    ESP_LOGD("LEDsService", "Constructor Start");

    REG_WRITE(GPIO_ENABLE1_REG, BIT0);

    REG_WRITE(GPIO_OUT1_W1TS_REG, BIT0);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    REG_WRITE(GPIO_OUT1_W1TC_REG, BIT0);

    cyclesoffset = xthal_get_ccount();
    while (((uint32_t)(xthal_get_ccount() - cyclesoffset)) < CYCLES_RESET)
        ;

    setLEDColor(0, 0x00, 0x00, 0x00);
    setLEDColor(1, 0x00, 0x00, 0x00);
    setLEDColor(2, 0x00, 0x00, 0x00);
    sendToLEDs();
    sendToLEDs();
    sendToLEDs();

    queueLedCommands = xQueueCreate(10, sizeof(ledCommand));

    ESP_LOGD("LEDsService", "Constructor END");

#ifndef ESP32_QEMU

#endif
}

void LEDsService::Run()
{
    ESP_LOGD("LEDsService", "Run");

    for (;;)
    {
        vTaskDelay(0);
        xQueueReceive(queueLedCommands, &ledCommand, portMAX_DELAY);

        ESP_LOGD(GetName().c_str(), "Run: ledCommand.effect = %d", ledCommand.effect);

        switch (ledCommand.effect)
        {
        case LED_EFFECT_SET:
            led_effect_set();
            break;

        case LED_EFFECT_BLINK:
            led_effect_blink();
            break;

        case LED_EFFECT_FADE:
            led_effect_fade();
            break;

        default:
            break;
        }

        // setLEDColor(0, (((float)esp_random() / (float)UINT32_MAX) * (float)255), (((float)esp_random() / (float)UINT32_MAX) * (float)255), (((float)esp_random() / (float)UINT32_MAX) * (float)255));
        // setLEDColor(1, (((float)esp_random() / (float)UINT32_MAX) * (float)255), (((float)esp_random() / (float)UINT32_MAX) * (float)255), (((float)esp_random() / (float)UINT32_MAX) * (float)255));
        // setLEDColor(2, (((float)esp_random() / (float)UINT32_MAX) * (float)255), (((float)esp_random() / (float)UINT32_MAX) * (float)255), (((float)esp_random() / (float)UINT32_MAX) * (float)255));
    }
}

esp_err_t LEDsService::queueCommand(led_command_t command)
{
    ESP_LOGD(GetName().c_str(), "queueCommand: command.effect = %d", command.effect);
    return xQueueSend(queueLedCommands, &command, portMAX_DELAY);
}

void LEDsService::led_effect_set()
{
    ESP_LOGD("LEDsService", "led_effect_set");

    for (size_t i = 0; i < NUM_LEDS; i++)
    {
        if (ledCommand.led[i] >= 0)
        {
            ESP_LOGD(GetName().c_str(), "led_effect_set: ledCommand.led[%d] = %d, R = %d, G = %d, B = %d", i, ledCommand.led[i], (*((uint8_t *)(&ledCommand.color) + 2)), (*((uint8_t *)(&ledCommand.color) + 1)), (*(uint8_t *)(&ledCommand.color)));
            setLEDColor(ledCommand.led[i], ledCommand.brightness * (*((uint8_t *)(&ledCommand.color) + 2)), ledCommand.brightness * (*((uint8_t *)(&ledCommand.color) + 1)), ledCommand.brightness * (*(uint8_t *)(&ledCommand.color)));
        }
    }

    sendToLEDs();
}
void LEDsService::led_effect_blink(){
    ESP_LOGD("LEDsService", "led_effect_blink");
    led_effect_set();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    for (size_t i = 0; i < NUM_LEDS; i++)
    {
        if (ledCommand.led[i] >= 0)
        {
            setLEDColor(ledCommand.led[i], 0, 0, 0);
        }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);

}

void LEDsService::led_effect_fade(){
    ESP_LOGD("LEDsService", "led_effect_fade");
    for(float intensity = 0; intensity <= ledCommand.brightness; intensity += 0.1)
    {
        for (size_t i = 0; i < NUM_LEDS; i++)
        {
            if (ledCommand.led[i] >= 0)
            {
                setLEDColor(ledCommand.led[i], intensity * (*((uint8_t *)(&ledCommand.color) + 2)), intensity * (*((uint8_t *)(&ledCommand.color) + 1)), intensity * (*(uint8_t *)(&ledCommand.color)));
            }
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    led_effect_set();

}

esp_err_t LEDsService::setLEDColor(uint8_t led, uint8_t red, uint8_t green, uint8_t blue)
{
    if (led < NUM_LEDS)
    {
        LEDs[led].red = red;
        LEDs[led].green = green;
        LEDs[led].blue = blue;
        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

esp_err_t LEDsService::sendToLEDs()
{
    uint32_t cyclesoffset = 0;
    uint32_t color[3] = {0, 0, 0};
    int bitpos = 0;
    bool bit = false;

    for (uint8_t i = 0; i < 3; i++)
        memcpy(&color[i], &LEDs[i], 3);

    ESP_LOGD("LEDsService", "sendToLEDs");

    vTaskSuspendAll();
    for (int i = 0; i < NUM_LEDS; i++)
    {
        for (bitpos = 23; bitpos >= 0; bitpos--)
        {
            bit = color[i] & (1 << bitpos);
            if (bit)
            {
                REG_WRITE(GPIO_OUT1_W1TS_REG, BIT0);
                cyclesoffset = xthal_get_ccount();
                while (((uint32_t)(xthal_get_ccount() - cyclesoffset)) < CYCLES_T1H)
                    ;
                REG_WRITE(GPIO_OUT1_W1TC_REG, BIT0);
            }
            else
            {
                REG_WRITE(GPIO_OUT1_W1TS_REG, BIT0);
                cyclesoffset = xthal_get_ccount();
                while (((uint32_t)(xthal_get_ccount() - cyclesoffset)) < CYCLES_T0H)
                    ;
                REG_WRITE(GPIO_OUT1_W1TC_REG, BIT0);
            }

            while (((uint32_t)(xthal_get_ccount() - cyclesoffset)) < CYCLES_PERIOD)
                ;
        }
    }
    xTaskResumeAll();

    cyclesoffset = xthal_get_ccount();
    while (((uint32_t)(xthal_get_ccount() - cyclesoffset)) < CYCLES_RESET)
        ;

    return ESP_OK;
}