#ifndef PID_SERVICE_H
#define PID_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

using namespace cpp_freertos;

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

class PIDService : public Thread
{
public:
    PIDService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    dataSpeed *speed;
    RobotStatus *status;
    dataPID *PIDTrans;
    dataPID *PIDRot;

    short const TaskDelay = 10; // 10ms
    float const BaseDeTempo = (TaskDelay * 1E-3);

    // Variaveis de calculo para os pids da velocidade rotacional e translacional
    float KpVel = 0, KiVel = 0, KdVel = 0;
    float KpRot = 0, KiRot = 0, KdRot = 0;

    //erros anteriores
    float errRot_ant = 0;   //errRot_ant2 = 0;
    float errTrans_ant = 0; //errTrans_ant2 = 0;

    //Variáveis para cálculo do pid rot e trans
    float PidTrans = 0;
    float Ptrans = 0, Itrans = 0, Dtrans = 0;
    float PidRot = 0;
    float Prot = 0, Irot = 0, Drot = 0;

    int iloop = 0;
};

#endif