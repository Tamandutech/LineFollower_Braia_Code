#ifndef ABSTRACT_SERVICE_H
#define ABSTRACT_SERVICE_H

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "RobotData.h"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

enum ExecStatus
{
    EXECSTATUS_STOPPED,
    EXECSTATUS_RUNNING,
    EXECSTATUS_FINISHED
};

class Service
{
public:
    Service(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);
    //virtual ~Service(){};

    virtual void Setup() = 0;
    virtual void Main() = 0;

    void create();
    void createAsync();

    void start();
    void startAsync();

protected:
    const char *name;
    Robot *robot = NULL;

private:
    ExecStatus setupStatus;
    ExecStatus mainStatus;

    TaskHandle_t xTaskService = NULL;
    uint32_t stackDepth;
    UBaseType_t priority;

    static void task(void *_params);
};

#endif