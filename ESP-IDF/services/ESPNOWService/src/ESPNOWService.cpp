#include "ESPNOWService.hpp"

ESPNOWService::ESPNOWService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    protocolHandler.EspNowInit(1, broadcastAddress, false);
};

void ESPNOWService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        uint8_t *ReceivedData;
        if (protocolHandler.dataAvailable())
        {
            ReceivedData = protocolHandler.getReceivedData();
        }

        protocolHandler.EspSend(MapDataSend, 0, 0, (void *)msgSend.c_str());
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
    }
}