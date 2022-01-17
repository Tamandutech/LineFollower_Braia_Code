#include "PIDService.h"

void PIDService::Main()
{
    // auto const TaskDelay = 10; // 10ms
    // auto const BaseDeTempo = (TaskDelay * 1E-3);
    // //auto const h1 = BaseDeTempo / 2;
    // //auto const h2 = 1 / BaseDeTempo;
    // //auto const h2x2 = h2 * 2;

    // Robot *braia = (Robot *)pvParameters;
    // dataSpeed *speed = braia->getSpeed();
    // RobotStatus *status = braia->getStatus();
    // dataPID *PIDTrans = braia->getPIDVel();
    // dataPID *PIDRot = braia->getPIDRot();

    // // Variaveis de calculo para os pids da velocidade rotacional e translacional
    // float KpVel = 0, KiVel = 0, KdVel = 0;
    // float KpRot = 0, KiRot = 0, KdRot = 0;

    // //erros anteriores
    // float errRot_ant = 0;   //errRot_ant2 = 0;
    // float errTrans_ant = 0; //errTrans_ant2 = 0;

    // //saidas pid anteriores
    // //float lastTransPid = 0, lastRotPid = 0;

    // //Variáveis para cálculo do pid rot e trans
    // float PidTrans = 0;
    // float Ptrans = 0, Itrans = 0, Dtrans = 0;
    // float PidRot = 0;
    // float Prot = 0, Irot = 0, Drot = 0;

    // // Definindo input da classe PID Rotacional valor de linha do sensor Array
    // PIDRot->setInput(braia->getsArray()->getLine());

    // ESP_LOGD("vTaskPID", "Pausando...");
    // vTaskSuspend(xTaskPID);

    // ESP_LOGD("vTaskPID", "Retomada!");

    // TickType_t xLastWakeTime = xTaskGetTickCount();
    // for (;;)
    // {
    //     CarState estado = status->getState();
    //     if (estado == CAR_IN_LINE)
    //         PIDTrans->setSetpoint(800);
    //     else if (estado == CAR_IN_CURVE)
    //         PIDTrans->setSetpoint(600);

    //     // Variaveis de calculo para os pids da velocidade rotacional e translacional
    //     KpVel = PIDTrans->getKp(estado);
    //     KiVel = PIDTrans->getKi(estado) * BaseDeTempo;
    //     KdVel = PIDTrans->getKd(estado) / BaseDeTempo;

    //     KpRot = PIDRot->getKp(estado);
    //     KiRot = PIDRot->getKi(estado) * BaseDeTempo;
    //     KdRot = PIDRot->getKd(estado) / BaseDeTempo;

    //     //Velocidade do carrinho
    //     float VelRot = speed->getRPMRight_inst() - speed->getRPMLeft_inst();   // Rotacional
    //     float VelTrans = speed->getRPMRight_inst() + speed->getRPMLeft_inst(); //Translacional

    //     //Erros atuais
    //     PIDRot->setSetpoint((3500 - braia->getsArray()->getLine()) / 7); // cálculo do setpoint rotacional
    //     float erroVelTrans = (float)(PIDTrans->getSetpoint()) - VelTrans;
    //     float erroVelRot = (float)(PIDRot->getSetpoint()) - VelRot;

    //     //calculando Pids rotacional e translacional
    //     //float PidTrans = erroVelTrans * (KpVel + KiVel * h1 + KdVel * h2) + errTrans_ant * (-KpVel + KiVel * h1 - KdVel * h2x2) + errTrans_ant2 * (KdVel * h2) + lastTransPid;
    //     //errTrans_ant2 = errTrans_ant;
    //     Ptrans = KpVel * erroVelTrans;
    //     Itrans += KiVel * erroVelTrans;
    //     Dtrans = KdVel * (erroVelTrans - errTrans_ant);
    //     PidTrans = Ptrans + Itrans + Dtrans;
    //     errTrans_ant = erroVelTrans;
    //     //lastTransPid = PidTrans;

    //     //float PidRot = erroVelRot * (KpRot + KiRot * h1 + KdRot * h2) + errRot_ant * (-KpRot + KiRot * h1 - KdRot * h2x2) + errRot_ant2 * (KdRot * h2) + lastRotPid;
    //     //errRot_ant2 = errRot_ant;
    //     Prot = KpRot * erroVelRot;
    //     Irot += KiRot * erroVelRot;
    //     Drot = KdRot * (erroVelRot - errRot_ant);
    //     PidRot = Prot + Irot + Drot;
    //     errRot_ant = erroVelRot;
    //     //lastRotPid = PidRot;

    //     auto speedBase = speed->getSpeedBase(estado);
    //     auto speedMin = speed->getSpeedMin(estado);
    //     auto speedMax = speed->getSpeedMax(estado);

    //     // PID output, resta adequar o valor do Pid para ficar dentro do limite do pwm
    //     PIDTrans->setOutput(constrain(
    //         ((PidTrans) + speedBase),
    //         speedMin,
    //         speedMax));

    //     PIDRot->setOutput(PidRot);

    //     // Calculo de velocidade do motor
    //     speed->setSpeedRight(
    //         constrain((int16_t)(PIDTrans->getOutput()) + (int16_t)(PIDRot->getOutput()), speedMin, speedMax),
    //         estado);

    //     speed->setSpeedLeft(
    //         constrain((int16_t)(PIDTrans->getOutput()) - (int16_t)(PIDRot->getOutput()), speedMin, speedMax),
    //         estado);

    //     ESP_LOGD("vTaskPID", "speedMin: %d | speedMax: %d", speedMin, speedMax);
    //     ESP_LOGD("vTaskPID", "PIDRot: %.2f | PIDTrans: %.2f", PIDRot->getOutput(), PIDTrans->getOutput());

    //     vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
    // }
}