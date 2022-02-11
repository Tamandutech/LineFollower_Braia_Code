#include "PIDService.hpp"

PIDService::PIDService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->speed = robot->getSpeed();
    this->status = robot->getStatus();
    this->PIDTrans = robot->getPIDVel();
    this->PIDRot = robot->getPIDRot();

    this->PIDRot->setInput(this->robot->getsArray()->getLine());
};

void PIDService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        CarState estado = status->getState();
        bool mapState = status->getMapping();

#if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG
        if (iloop > 50)
        {
            ESP_LOGD(GetName().c_str(), "CarstatusOut: %d | bool : %d", estado, mapState);
            ESP_LOGD(GetName().c_str(), "SetPointTrans: %d", PIDTrans->getSetpoint());
            iloop = 0;
        }
        iloop++;
#endif

        // Altera a velocidade linear do carrinho
        if (estado == CAR_IN_LINE && !mapState)
        {
            PIDTrans->setSetpoint(1800);
        }
        else if (estado == CAR_IN_CURVE && !mapState)
        {
            PIDTrans->setSetpoint(200);
        }
        else if(mapState && estado != CAR_STOPPED){
            PIDTrans->setSetpoint(100);
        }
        
        // Reseta o PID se o carrinho parar
        if(estado == CAR_STOPPED){
            Ptrans = 0;
            Dtrans = 0;
            Itrans = 0;
            Prot = 0;
            Drot = 0;
            Irot = 0;
        }
        // Variaveis de calculo para os pids da velocidade rotacional e translacional
        KpVel = PIDTrans->getKp(estado);
        KiVel = PIDTrans->getKi(estado) * BaseDeTempo;
        KdVel = PIDTrans->getKd(estado) / BaseDeTempo;

        KpRot = PIDRot->getKp(estado);
        KiRot = PIDRot->getKi(estado) * BaseDeTempo;
        KdRot = PIDRot->getKd(estado) / BaseDeTempo;

        //Velocidade do carrinho
        float VelRot = speed->getRPMRight_inst() - speed->getRPMLeft_inst();   // Rotacional
        float VelTrans = speed->getRPMRight_inst() + speed->getRPMLeft_inst(); //Translacional

        //Erros atuais
        PIDRot->setSetpoint((3500 - robot->getsArray()->getLine()) / 7); // cálculo do setpoint rotacional
        float erroVelTrans = (float)(PIDTrans->getSetpoint()) - VelTrans;
        float erroVelRot = (float)(PIDRot->getSetpoint()) - VelRot;

        //calculando Pids rotacional e translacional
        //float PidTrans = erroVelTrans * (KpVel + KiVel * h1 + KdVel * h2) + errTrans_ant * (-KpVel + KiVel * h1 - KdVel * h2x2) + errTrans_ant2 * (KdVel * h2) + lastTransPid;
        //errTrans_ant2 = errTrans_ant;
        Ptrans = KpVel * erroVelTrans;
        Itrans += KiVel * erroVelTrans;
        Dtrans = KdVel * (erroVelTrans - errTrans_ant);
        PidTrans = Ptrans + Itrans + Dtrans;
        errTrans_ant = erroVelTrans;
        //lastTransPid = PidTrans;

        //float PidRot = erroVelRot * (KpRot + KiRot * h1 + KdRot * h2) + errRot_ant * (-KpRot + KiRot * h1 - KdRot * h2x2) + errRot_ant2 * (KdRot * h2) + lastRotPid;
        //errRot_ant2 = errRot_ant;
        Prot = KpRot * erroVelRot;
        Irot += KiRot * erroVelRot;
        Drot = KdRot * (erroVelRot - errRot_ant);
        PidRot = Prot + Irot + Drot;
        errRot_ant = erroVelRot;
        //lastRotPid = PidRot;

        auto speedBase = speed->getSpeedBase(estado);
        auto speedMin = speed->getSpeedMin(estado);
        auto speedMax = speed->getSpeedMax(estado);

        // PID output, resta adequar o valor do Pid para ficar dentro do limite do pwm
        PIDTrans->setOutput(constrain(
            ((PidTrans) + speedBase),
            speedMin,
            speedMax));

        PIDRot->setOutput(PidRot);

        // Calculo de velocidade do motor
        speed->setSpeedRight(
            constrain((int16_t)(PIDTrans->getOutput()) + (int16_t)(PIDRot->getOutput()), speedMin, speedMax),
            estado);

        speed->setSpeedLeft(
            constrain((int16_t)(PIDTrans->getOutput()) - (int16_t)(PIDRot->getOutput()), speedMin, speedMax),
            estado);

        ESP_LOGD(GetName().c_str(), "speedMin: %d | speedMax: %d", speedMin, speedMax);
        ESP_LOGD(GetName().c_str(), "PIDRot: %.2f | PIDTrans: %.2f", PIDRot->getOutput(), PIDTrans->getOutput());

        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
    }
}