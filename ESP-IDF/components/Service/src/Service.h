#ifndef ABSTRACT_SERVICE_H
#define ABSTRACT_SERVICE_H

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "RobotData.h"

class Service
{
public:
    Service(std::string name, uint32_t stackDepth, UBaseType_t priority);
    virtual ~Service();

    virtual void Setup() = 0;
    virtual void Main() = 0;

protected:
    std::string name;
    TaskHandle_t xTaskService;

    static void task(void *_params);
};

#endif