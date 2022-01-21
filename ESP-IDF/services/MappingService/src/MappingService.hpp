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

    int32_t mappingData[3][40] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}; // [tempo][media][estado]
                                                                    //  "quantidade de marcações"
    uint16_t marks = 0;
};

#endif