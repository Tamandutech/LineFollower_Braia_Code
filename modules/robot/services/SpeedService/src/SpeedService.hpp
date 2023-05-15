#ifndef SPEED_SERVICE_H
#define SPEED_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"

#include "ESP32Encoder.h"
#include "math.h" 

using namespace cpp_freertos;

#include "esp_log.h"

class SpeedService : public Thread, public Singleton<SpeedService>
{
public:
    SpeedService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    dataSpeed *speed;

    CarState estado;

    // Componente de gerenciamento dos encoders
    ESP32Encoder enc_motEsq;
    ESP32Encoder enc_motDir;
    
    short const TaskDelay = 10; // 10 ms
    uint16_t MPR_Mot = 180;
    TickType_t lastTicksRevsCalc = 0;
    int32_t lastPulseRight = 0;
    int32_t lastPulseLeft = 0;
    uint16_t deltaTimeMS_inst = 0; // delta entre ultimo calculo e o atual em millisegundos

    float deltaS = 0;
    float deltaA = 0;
    float deltaEncEsq = 0;
    float deltaEncDir = 0;
    float diameterWheel = 0;
    float diameterRobot = 0;
    float positionX = 0, positionY = 0;
    float DeltaPositionX = 0, DeltaPositionY = 0;


    TickType_t initialTicksCar = 0;
    uint16_t deltaTimeMS_media = 0;
    int iloop=0;
};

#endif