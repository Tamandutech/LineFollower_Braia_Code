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
    bool getMapFinished(); // informa se o mapeamento está finalizado
    uint16_t getTotalLeftMarks(); // Número total de marcações laterais esquerda da pista 
    int32_t getFinalMark(); // Média dos encoders da marcação final
    int32_t getInitialMark(); // Média dos encoders da marcação inicial
    struct MapData getMarkDataReg(int regnum); // Dados das marcações laterais da pista
    int SetSLatEsq(bool latesqPass);
    int SetSLatDir(bool latdirPass);
    int SetMapFinished(bool mapfinished);
    int SetTotalLeftMarks(uint16_t totalmarks);
    int SetInitialMark(int32_t initialmark);
    int SetFinalMark(int32_t finalmark);
    int SetMarkDataReg(struct MapData markreg, int regnum);
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
    struct MapData MarksData[70];
    SemaphoreHandle_t xSemaphoreMarksData;
    uint16_t TotalLeftMarks = 0;
    SemaphoreHandle_t xSemaphoreTotalLeftMarks;
    int32_t InitialMark = 0;
    SemaphoreHandle_t xSemaphoreInitialMark;
    int32_t FinalMark = 0;
    SemaphoreHandle_t xSemaphoreFinalMark;
    bool MapFinished = false;
    SemaphoreHandle_t xSemaphoreMapFinished;

};

#endif