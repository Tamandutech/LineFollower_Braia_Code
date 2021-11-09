#ifndef DATA_SLATMARKS_H
#define DATA_SLATMARKS_H

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

class dataSLatMarks
{
public:
    dataSLatMarks(std::string name = "dataSensor");

    

private:
    std::string name;
    const char *tag = "dataSLatMarks";
    SemaphoreHandle_t xSemaphorelatesqPass;
    bool latesqPass  = false;
    SemaphoreHandle_t xSemaphorelatdirPass;
    bool latdirPass  = false;

};

#endif