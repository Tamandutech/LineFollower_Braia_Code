#ifndef ABSTRACT_SERVICE_H
#define ABSTRACT_SERVICE_H

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "RobotData.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

class Service
{
public:
    Service(const char* name, uint32_t stackDepth, UBaseType_t priority);
    virtual ~Service(){};

    virtual void Setup() = 0;
    virtual void Main() = 0;

    void create();

protected:
    const char* name;
    TaskHandle_t xTaskService;
    uint32_t stackDepth;
    UBaseType_t priority;

    static void task(void *_params);
};

#endif