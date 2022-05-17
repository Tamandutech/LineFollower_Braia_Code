#include "LEDsService.hpp"

LEDsService::LEDsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->status = robot->getStatus();
    uint8_t *colorsRGB = (uint8_t *) malloc(3);
    REG_WRITE(GPIO_ENABLE_REG, BIT32);
    status->ColorLed0->setData(0xFFFFFF);
    status->ColorLed1->setData(0xFFFFFF);
    status->ColorLed2->setData(0xFFFFFF);

#ifndef ESP32_QEMU

#endif
}

void LEDsService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
#ifndef ESP32_QEMU

#endif
        if (status->robotIsMapping->getData() || status->encreading->getData())
        {
            if (status->robotState->getData() == CAR_IN_LINE)
            {
                status->ColorLed0->setData(0xFF0000);
            }
            else if (status->robotState->getData() == CAR_IN_CURVE)
            {
                status->ColorLed0->setData(0x00FF00);
            }
        }
        uint32_t colors0 = status->ColorLed0->getData();
        memcpy(colorsRGB, &colors0, 3);
        setLEDColor(0, colorsRGB[2], colorsRGB[1], colorsRGB[0]);
        uint32_t colors1 = status->ColorLed1->getData();
        memcpy(colorsRGB, &colors1, 3);
        setLEDColor(1, colorsRGB[2], colorsRGB[1], colorsRGB[0]);
        uint32_t colors2 = status->ColorLed2->getData();
        memcpy(colorsRGB, &colors2, 3);
        setLEDColor(2, colorsRGB[2], colorsRGB[1], colorsRGB[0]);
        sendToLEDs();
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
    }
}

esp_err_t LEDsService::setLEDColor(uint8_t led, uint8_t red, uint8_t green, uint8_t blue){
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

esp_err_t LEDsService::sendToLEDs(){
    uint32_t cyclesoffset = 0;
    uint32_t color;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        memcpy(&color, &LEDs[i], 3);
        for (int bitpos = 0; bitpos < 24; bitpos++)
        {
            bool bit = color & (1 << bitpos);
            if (bit)
            {
                REG_WRITE(GPIO_OUT_W1TS_REG, BIT32);
                cyclesoffset = xthal_get_ccount();
                while(((uint32_t)(xthal_get_ccount() - cyclesoffset)) < CYCLES_800_T1H);
                REG_WRITE(GPIO_OUT_W1TC_REG, BIT32);
            }
            else
            {
                REG_WRITE(GPIO_OUT_W1TS_REG, BIT32);
                cyclesoffset = xthal_get_ccount();
                while(((uint32_t)(xthal_get_ccount() - cyclesoffset)) < CYCLES_800_T0H);
                REG_WRITE(GPIO_OUT_W1TC_REG, BIT32);
            }
            while(xthal_get_ccount() - cyclesoffset < CYCLES_800);
        }
        vTaskDelay(1);
    }
   return ESP_OK;
}