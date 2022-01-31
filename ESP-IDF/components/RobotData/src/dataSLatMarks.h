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

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

class dataSLatMarks
{
public:
    dataSLatMarks(std::string name = "dataSensor");
    bool getSLatEsq();
    bool getSLatDir();
    int SetSLatEsq(bool latesqPass);
    int SetSLatDir(bool latdirPass);
    int leftPassedInc();
    int rightPassedInc();
    uint16_t getleftMarks();
    uint16_t getrightMarks();


    

private:
    std::string name;
    const char *tag = "dataSLatMarks";
    SemaphoreHandle_t xSemaphorelatesqPass;
    bool latesqPass  = false;
    SemaphoreHandle_t xSemaphorelatdirPass;
    bool latdirPass  = false;
    uint16_t leftMarks = 0;
    SemaphoreHandle_t xSemaphoreleftMarks;
    uint16_t rightMarks = 0;
    SemaphoreHandle_t xSemaphorerightMarks;

};

#endif