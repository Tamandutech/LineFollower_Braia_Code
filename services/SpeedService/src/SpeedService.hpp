#ifndef SPEED_SERVICE_H
#define SPEED_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "ESP32Encoder.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

class SpeedService : public Thread
{
public:
    SpeedService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    dataSpeed *speed;

    // Componente de gerenciamento dos encoders
    ESP32Encoder enc_motEsq;
    ESP32Encoder enc_motDir;

    short const TaskDelay = 10; // 10 ms
    uint16_t const MPR_MotEsq = 120;
    uint16_t const MPR_MotDir = 120;

    TickType_t lastTicksRevsCalc = 0;
    int32_t lastPulseRight = 0;
    int32_t lastPulseLeft = 0;
    uint16_t deltaTimeMS_inst = 0; // delta entre ultimo calculo e o atual em millisegundos

    TickType_t initialTicksCar = 0;
    uint16_t deltaTimeMS_media = 0;
    int iloop=0;
};

#endif