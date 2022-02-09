#ifndef MAPPING_SERVICE_H
#define MAPPING_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "driver/gpio.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

class MappingService : public Thread
{
public:
    MappingService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    dataSpeed *speedMapping;

    dataSensor *SLat;
    dataSLatMarks *latMarks;

    struct MapData markreg;

    bool startTimer = false;
    bool mapfinish = false;
    bool leftpassed = false;

    int iloop = 0;               // Variável para debug
    
    int32_t FinalMarkData = 0;   // Media dos encoders na marcação final
    int32_t InitialMarkData = 0; // Media dos encoders na marcação inicial
                                                                    //  "quantidade de marcações"
    uint16_t marks = 0;
};

#endif