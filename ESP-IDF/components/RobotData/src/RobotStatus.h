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

class RobotStatus
{
public:
    RobotStatus(CarState initialState, std::string name);

    int setState(CarState actualState);
    CarState getState();

private:
    std::string name;
    const char *tag = "RobotStatus";

    CarState robotState;
    SemaphoreHandle_t xSemaphoreRobotState;
};

#endif