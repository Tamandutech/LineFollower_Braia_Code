#ifndef PID_SERVICE_H
#define PID_SERVICE_H

#include <math.h>
#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"
#include "dataEnums.h"

using namespace cpp_freertos;

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

//#define GRAPH_DATA
#include "esp_log.h"

class PIDService : public Thread, public Singleton<PIDService>
{
public:
    PIDService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    dataSpeed *speed;
    RobotStatus *status;
    dataPID *PIDTrans;
    dataPID *PIDRot;

    short const TaskDelay = 10; // 10ms

    // Variaveis de calculo para os pids da velocidade rotacional e translacional
    float KpVel = 0, KiVel = 0, KdVel = 0;
    float KpRot = 0, KiRot = 0, KdRot = 0;

    // erros anteriores
    float errRot_ant = 0;   // errRot_ant2 = 0;
    float errTrans_ant = 0; // errTrans_ant2 = 0;

    // Variáveis para cálculo do pid rot e trans
    float rotK = 5;
    float accel = 6000; // aceleração em mm/s^2
    float desaccel = 6000; // aceleração em mm/s^2
    int16_t setpointPIDTransTarget = 0;
    int16_t newSetpoint = 0;
    int16_t SetpointTransactual = 0;
    float PidTrans = 0;
    float Ptrans = 0, Itrans = 0, Dtrans = 0;
    float PidRot = 0;
    float Prot = 0, Irot = 0, Drot = 0;

    int8_t speedBase = 0;
    int8_t speedMin = 0;
    int8_t speedMax = 0;

    float VelRot = 0;
    float VelTrans = 0;
    float lastVelRot = 0;
    float lastVelTrans = 0;

    float erroVelTrans = 0;
    float erroVelRot = 0;

    bool mapState = false;

    CarState estado = CAR_STOPPED;
    TrackState TrackLen = SHORT_CURVE;

    int iloop = 0;
    int gloop = 0; // taxa de atualização dos dados para a plotagem de gráficos 
};

#endif