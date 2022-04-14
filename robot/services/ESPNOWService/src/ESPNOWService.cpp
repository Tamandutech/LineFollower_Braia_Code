#include "ESPNOWService.hpp"

ESPNOWService::ESPNOWService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    status = robot->getStatus();

    dataManager = dataManager->getInstance();

    protocolHandler = protocolHandler->getInstance();
};

void ESPNOWService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
       
    

        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
    }
}