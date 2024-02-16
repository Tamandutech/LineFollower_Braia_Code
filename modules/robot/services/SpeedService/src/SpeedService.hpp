#ifndef SPEED_SERVICE_H
#define SPEED_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"

#include "ESP32Encoder.h"
#include "math.h" 

using namespace cpp_freertos;

#define MICROSECONDS_TO_MINUTES_RATIO 6.0E7

#include "esp_log.h"

class SpeedService : public Thread, public Singleton<SpeedService>
{
public:
    SpeedService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

    void resetEncondersValue();
    void MeasureWheelsSpeed();
    int16_t CalculateRobotLinearSpeed();
    int16_t CalculateOffsetToDecelerate(int16_t FinalSpeed, float DecelerationAdjustableGain);


private:
    Robot *robot;
    dataSpeed *speed;

    CarState estado;

    // Componente de gerenciamento dos encoders
    ESP32Encoder enc_motEsq;
    ESP32Encoder enc_motDir;
    
    short const TaskDelay = 10; // 10 ms
    uint16_t MPR_Mot = 180;
    int32_t lastPulseLeftToPositionCalculus = 0;
    int32_t lastPulseRightToPositionCalculus = 0;
    int32_t lastPulseRight = 0;
    int32_t lastPulseLeft = 0;
    int64_t lastTimeWheelsSpeedMeasured = 0;
    std::mutex EncodersMutex;
    
    float deltaS = 0;
    float deltaA = 0; // rad/s
    float Ang = 0; // rad/s
    float deltaEncEsq = 0;
    float deltaEncDir = 0;
    float diameterWheel = 0; // mm
    float diameterRobot = 0; // mm
    float positionX = 0, positionY = 0; // mm
    float DeltaPositionX = 0, DeltaPositionY = 0; // mm


    TickType_t initialTicksCar = 0;
    uint16_t deltaTimeMS_media = 0;
    int iloop=0;

    int16_t CalculateWheelSpeed(int32_t ActualPulsesCount, int32_t lastPulsesCount, int64_t dt_MicroSeconds);
    void storeWheelsSpeed(int16_t LeftWheelSpeed,int16_t RightWheelSpeed);
    void storeEncCount(int16_t LeftWheelCount,int16_t RightWheelCount);

};

#endif