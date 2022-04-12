#ifndef DATA_SLATMARKS_H
#define DATA_SLATMARKS_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <cstring>

#include "dataEnums.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "DataAbstract.hpp"
#include "DataStorage.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

class dataSLatMarks
{
public:
    dataSLatMarks(std::string name = "dataSensor");

    DataAbstract<bool> *latEsqPass;
    DataAbstract<bool> *latDirPass;
    DataAbstract<uint16_t> *leftMarks;
    DataAbstract<uint16_t> *rightMarks;
    DataAbstract<uint16_t> *totalLeftMarks;
    DataAbstract<int32_t> *initialMark;
    DataAbstract<int32_t> *finalMark;
    DataAbstract<bool> *mapFinished;

    struct MapData getMarkDataReg(int regnum); // Dados das marcações laterais da pista
    int SetMarkDataReg(struct MapData markreg, int regnum);

    void leftPassedInc();
    void rightPassedInc();

    int setData(struct SLatMarks SLatData);
    struct SLatMarks getData();

private:
    std::string name;
    const char *tag = "dataSLatMarks";

    SemaphoreHandle_t xSemaphoreMarksData;

    struct MapData MarksData[70]; // salvar
};

#endif