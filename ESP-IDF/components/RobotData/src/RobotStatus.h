#ifndef ROBOT_STATUS_H
#define ROBOT_STATUS_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>

#include "dataEnums.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

class RobotStatus
{
public:
    RobotStatus(CarState initialState, std::string name);

    int setState(CarState actualState);
    CarState getState();
    
    int setMapping(bool value);
    bool getMapping();

private:
    std::string name;
    const char *tag = "RobotStatus";

    CarState robotState = CAR_IN_LINE;
    SemaphoreHandle_t xSemaphoreRobotState;
    bool robotMap;
    SemaphoreHandle_t xSemaphoreRobotMap;
};

#endif