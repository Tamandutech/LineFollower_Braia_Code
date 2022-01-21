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
        if (estado == CAR_IN_LINE)
            PIDTrans->setSetpoint(800);
        else if (estado == CAR_IN_CURVE)
            PIDTrans->setSetpoint(600);

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
        Ptrans = KpVel * erroVelTrans;
        Itrans += KiVel * erroVelTrans;
        Dtrans = KdVel * (erroVelTrans - errTrans_ant);
        PidTrans = Ptrans + Itrans + Dtrans;
        errTrans_ant = erroVelTrans;

        Prot = KpRot * erroVelRot;
        Irot += KiRot * erroVelRot;
        Drot = KdRot * (erroVelRot - errRot_ant);
        PidRot = Prot + Irot + Drot;
        errRot_ant = erroVelRot;

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