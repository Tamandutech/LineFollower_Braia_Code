#ifndef PID_SERVICE_H
#define PID_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"
#include "dataEnums.h"
#include "SensorsService.hpp"

#include "TrackSegment.hpp"
#include "TypePID.hpp"

#include "ESP32MotorControl.h"

// Timer control
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "driver/gptimer.h"
#include "esp_log.h"

using namespace cpp_freertos;

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

#define ARRAY_TARGET_POSITION 3500
#define MAX_SPEED 100
#define MIN_SPEED -100

#define TIMER_FREQ 1000000
#include "esp_log.h"


class PIDService : public Thread, public Singleton<PIDService>
{
public:
    PIDService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void ControlMotors(float left, float right);

    // Timer control
    static bool IRAM_ATTR timer_group_isr_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);

    void Run() override;

private:
    Robot *robot;
    dataSpeed *speed;
    RobotStatus *status;
    dataPID *DataPID;

    ESP32MotorControl motors;

    const short TaskDelay = 5; // 5ms
    const float TaskDelaySeconds = TaskDelay / 1000.0;

    float accel = 300;    // aceleração em porcentagem
    float desaccel = 300; // aceleração em porcentagem
    float speedTarget = 0;

    // Variáveis para cálculo do pid
    float lastSensorArrayPosition = 0;

    int8_t speedBase = 0;
    int8_t speedMin = 0;
    int8_t speedMax = 0;

    float soma_erro = 0;

    bool mapState = false;

    CarState estado = CAR_STOPPED;
    TrackSegment RealTracklen = TrackSegment::SHORT_LINE;

    int iloop = 0;

    // Timer control
    static SemaphoreHandle_t SemaphoreTimer; // semáforo para sincronização do timer com a task timer

    // Protótipos de função
    float calculatePID(PID_Consts pidConsts, float erro, float somaErro, float input, float *lastInput);
    void resetGlobalVariables();
    float calculateSpeed(float acceleration, float speedValue);
    void storingSpeedValue(float newSpeed);
};

#endif