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
        if (protocolHandler.dataAvailable())
        {
            packet = protocolHandler.getPacketReceived();
            if(packet.packetsToReceive==0){
                memcpy(msgrecebida,packet.data,packet.size);
                ESP_LOGD(GetName().c_str(), "Mensagem: %s",msgrecebida);
                ESP_LOGD(GetName().c_str(), "Tamanho: %d",packet.size);   
            }

        }

        protocolHandler.EspSend(CMDTXT, 2, sizeof(msgSend), (void *)msgSend);
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
    }
}