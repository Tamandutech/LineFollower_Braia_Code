#include "PIDService.hpp"

PIDService::PIDService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->speed = robot->getSpeed();
    this->status = robot->getStatus();
    this->PIDTrans = robot->getPIDVel();
    this->PIDRot = robot->getPIDRot();

    this->PIDRot->input->setData(this->robot->getsArray()->getLine());
};

void PIDService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        CarState estado = status->robotState->getData();
        bool mapState = status->robotMap->getData();

#if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG
        if (iloop > 50)
        {
            ESP_LOGD(GetName().c_str(), "CarstatusOut: %d | bool : %d", estado, mapState);
            ESP_LOGD(GetName().c_str(), "SetPointTrans: %d", PIDTrans->setpoint->getData());
            iloop = 0;
        }
        iloop++;
#endif

        // Altera a velocidade linear do carrinho
        if (estado == CAR_IN_LINE && !mapState)
        {
            PIDTrans->setpoint->setData(900);
        }
        else if (estado == CAR_IN_CURVE && !mapState)
        {
            PIDTrans->setpoint->setData(1000);
        }
        else if(mapState && estado != CAR_STOPPED){
            PIDTrans->setpoint->setData(1000);
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
        KpVel = PIDTrans->Kp(estado)->getData();
        KiVel = PIDTrans->Ki(estado)->getData() * BaseDeTempo;
        KdVel = PIDTrans->Kd(estado)->getData() / BaseDeTempo;

        KpRot = PIDRot->Kp(estado)->getData();
        KiRot = PIDRot->Ki(estado)->getData() * BaseDeTempo;
        KdRot = PIDRot->Kd(estado)->getData() / BaseDeTempo;

        //Velocidade do carrinho
        float VelRot = speed->getRPMRight_inst() - speed->getRPMLeft_inst();   // Rotacional
        float VelTrans = speed->getRPMRight_inst() + speed->getRPMLeft_inst(); //Translacional

        //Erros atuais
        PIDRot->setpoint->setData((3500 - robot->getsArray()->getLine()) / 7); // cálculo do setpoint rotacional
        float erroVelTrans = (float)(PIDTrans->setpoint->getData()) - VelTrans;
        float erroVelRot = (float)(PIDRot->setpoint->getData()) - VelRot;

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
        PIDTrans->output->setData(constrain(
            ((PidTrans) + speedBase),
            speedMin,
            speedMax));

        PIDRot->output->setData(PidRot);

        // Calculo de velocidade do motor
        speed->setSpeedRight(
            constrain((int16_t)(PIDTrans->output->getData()) + (int16_t)(PIDRot->output->getData()), speedMin, speedMax),
            estado);

        speed->setSpeedLeft(
            constrain((int16_t)(PIDTrans->output->getData()) - (int16_t)(PIDRot->output->getData()), speedMin, speedMax),
            estado);

        ESP_LOGD(GetName().c_str(), "speedMin: %d | speedMax: %d", speedMin, speedMax);
        ESP_LOGD(GetName().c_str(), "PIDRot: %.2f | PIDTrans: %.2f", PIDRot->output->getData(), PIDTrans->output->getData());

        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
    }
}