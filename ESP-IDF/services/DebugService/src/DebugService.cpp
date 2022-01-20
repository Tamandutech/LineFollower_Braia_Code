#include "DebugService.h"

void DebugService::Main()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        ESP_LOGD(name, "Rodando Main suavão, debugString: %s", debugString.c_str());
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 2000 / portTICK_PERIOD_MS);
    }
}

void DebugService::Setup()
{
    debugString = "Braia";
    ESP_LOGD(name, "Setup OK, debugString: %s", debugString.c_str());
    
}