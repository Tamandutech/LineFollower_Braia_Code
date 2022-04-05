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

#include "DataAbstract.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

class RobotStatus
{
public:
    RobotStatus(CarState initialState, std::string name);

    DataAbstract<CarState> *robotState;
    DataAbstract<bool> *robotMap;

private:
    std::string name;
    const char *tag = "RobotStatus";
};

#endif