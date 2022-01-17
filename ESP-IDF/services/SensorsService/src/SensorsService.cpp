#include "SensorsService.h"

void SensorsService::Main()
{
    // static const char *TAG = "vTaskSensors";
    // Robot *braia = (Robot *)pvParameters;

    // // Setup
    // ESP_LOGD(TAG, "Task criada!");

    // // Componente de gerenciamento dos sensores
    // QTRSensors sArray;
    // QTRSensors sLat;

    // // Definindo GPIOs e configs para sensor Array
    // sArray.setTypeMCP3008();
    // sArray.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, 8, (gpio_num_t)ADC_DOUT, (gpio_num_t)ADC_DIN, (gpio_num_t)ADC_CLK, (gpio_num_t)ADC_CS, 1350000, VSPI_HOST);
    // sArray.setSamplesPerSensor(5);

    // // Definindo GPIOs e configs para sensor Lateral
    // gpio_pad_select_gpio(17);
    // gpio_set_direction(GPIO_NUM_17, GPIO_MODE_INPUT);
    // gpio_pad_select_gpio(05);
    // gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);
    // sLat.setTypeAnalogESP();
    // sLat.setSensorPins((const adc1_channel_t[]){SL1, SL2}, 2);
    // sLat.setSamplesPerSensor(5);

    // calibAllsensors(&sArray, &sLat, braia); // calibração dos sensores

    // vTaskResume(xTaskMotors);
    // vTaskResume(xTaskPID);
    // if (taskStatus)
    //     vTaskResume(xTaskCarStatus);
    // vTaskResume(xTaskSpeed);
    // if (braia->getStatus()->getMapping())
    //     vTaskResume(xTaskMapping);

    // ESP_LOGD(TAG, "Retomada!");

    // // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
    // TickType_t xLastWakeTime = xTaskGetTickCount();

    // // Loop
    // for (;;)
    // {
    //     getSensors(&sArray, &sLat, braia); // leitura dos sensores
    //     processSLat(braia);

    //     vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
    // }
}