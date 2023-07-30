#ifndef PID_SERVICE_H
#define PID_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"
#include "dataEnums.h"
#include "SensorsService.hpp"

#include "ESP32MotorControl.h"

// Timer control
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "driver/timer.h"
#include "esp_log.h"

using namespace cpp_freertos;

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// #define GRAPH_DATA
#include "esp_log.h"

class PIDService : public Thread, public Singleton<PIDService>
{
public:
    PIDService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void ControlMotors(float left, float right);

    // Timer control
    static bool IRAM_ATTR timer_group_isr_callback(void * args);
    void vTaskFunction(void *pvParameters);
    void vTaskTimerFunction(void *pvParameters)


    void Run() override;

private:
    Robot *robot;
    dataSpeed *speed;
    RobotStatus *status;
    dataPID *PIDTrans;
    dataPID *PIDRot;
    dataPID *PIDIR;
    dataPID *PIDClassic;

    ESP32MotorControl motors;

    const short TaskDelay = 1; // 1ms
    const float TaskDelaySeconds = TaskDelay / 1000.0;

    bool pid_select = false;

    // Variaveis de calculo para os pids da velocidade rotacional e translacional
    double KpVel = 0, KiVel = 0, KdVel = 0;
    double KpRot = 0, KiRot = 0, KdRot = 0;
    double KpIR = 0, KdIR = 0, KiIR = 0;

    // erros anteriores
    float errRot_ant = 0;   // errRot_ant2 = 0;
    float errTrans_ant = 0; // errTrans_ant2 = 0;

    // Variáveis para cálculo dos pids
    float accel = 6000;    // aceleração em rpm/s ou em porcentagem
    float desaccel = 6000; // aceleração em rpm/s ou em porcentagem
    float speedTarget = 0;
    float newSpeed = 0;
    float calculatedSpeed = 0; // Velocidade calculada com base na aceleração
    float PidTrans = 0;
    float Ptrans = 0, Itrans = 0, Dtrans = 0;
    float P_IR = 0, I_IR = 0, D_IR = 0;
    float IR = 0; // posição do robô na linha;
    float PidRot = 0, PidIR = 0;
    float Prot = 0, Irot = 0, Drot = 0;

    int8_t speedBase = 0;
    int8_t speedMin = 0;
    int8_t speedMax = 0;

    float fatorCorrecao = 0; // taxa de redução de velocidade proporcional ao erro

    float VelRot = 0;
    float VelTrans = 0;

    // Tunning PID
    float lastIR = 0;
    float lastVelRot = 0;
    float lastVelTrans = 0;
    float lastPIDTrans = 0.0;
    float lastPIDRot = 0.0;
    float lastPIDIR = 0.0;
    double alphaVel = 0.00000000002; // Taxa de aprendizagem de 0 ate 1
    double alphaRot = 0.00000000002; // Taxa de aprendizagem de 0 ate 1
    double alphaIR = 0.0000000002;

    float erroVelTrans = 0;
    float erroVelRot = 0;
    float erroIR = 0;
    float soma_erroVelTrans = 0;
    float soma_erroVelRot = 0;
    float soma_erroIR = 0;

    bool mapState = false;

    CarState estado = CAR_STOPPED;
    TrackState TrackLen = SHORT_LINE, RealTracklen = SHORT_LINE;

    int iloop = 0;
    int gloop = 0; // taxa de atualização dos dados para a plotagem de gráficos

    // Timer control
    int SharedData = 0;
    SemaphoreHandle_t SharedSemaphore; // semáforo para os dados compartilhados das tasks
    SemaphoreHandle_t SemaphoreTimer;  // semáforo para sincronização do timer com a task timer
};

#endif