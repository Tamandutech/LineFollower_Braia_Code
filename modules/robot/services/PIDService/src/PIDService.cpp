#include "PIDService.hpp"

PIDService::PIDService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();
    ;
    this->speed = robot->getSpeed();
    this->status = robot->getStatus();
    this->PIDTrans = robot->getPIDVel();
    this->PIDRot = robot->getPIDRot();

    this->PIDRot->input->setData(this->robot->getsArray()->getLine());
    PIDTrans->setpoint->setData(0);
    setpointPIDTransTarget = 0;
};

void PIDService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);

        estado = (CarState)status->robotState->getData();
        // mapState = status->robotIsMapping->getData();

        speedBase = speed->base->getData();
        speedMin = speed->min->getData();
        speedMax = speed->max->getData();

        accel = speed->accelration->getData();
        rotK = PIDRot->Krot->getData();

        // Reseta o PID se o carrinho parar
        if (estado == CAR_STOPPED)
        {
            Ptrans = 0;
            Dtrans = 0;
            Itrans = 0;
            Prot = 0;
            Drot = 0;
            Irot = 0;
            PIDTrans->setpoint->setData(0);
            setpointPIDTransTarget = 0;
        }

        // Variaveis de calculo para os pids da velocidade rotacional e translacional
        KpVel = PIDTrans->Kp(estado)->getData();
        KiVel = PIDTrans->Ki(estado)->getData();
        KdVel = PIDTrans->Kd(estado)->getData();

        KpRot = PIDRot->Kp(estado)->getData();
        KiRot = PIDRot->Ki(estado)->getData();
        KdRot = PIDRot->Kd(estado)->getData();

        // Velocidade do carrinho
        VelRot = speed->RPMRight_inst->getData() - speed->RPMLeft_inst->getData();   // Rotacional
        VelTrans = speed->RPMRight_inst->getData() + speed->RPMLeft_inst->getData(); // Translacional

        // Erros atuais
        // rotK (porcentagem do erro do PID rotacioanl que representará a variação máxima de RPM dos motores)
        PIDRot->setpoint->setData((3500 - robot->getsArray()->getLine()) / rotK); // cálculo do setpoint rotacional
        erroVelTrans = (float)(PIDTrans->setpoint->getData()) - VelTrans;
        if (iloop > 100)
        {
            ESP_LOGD(GetName().c_str(), "PIDTrans->setpoint: %d", PIDTrans->setpoint->getData());
        }
        erroVelRot = (float)(PIDRot->setpoint->getData()) - VelRot;
        // erroVelRot = 3500 - robot->getsArray()->getLine();

        // calculando Pids rotacional e translacional
        Ptrans = KpVel * erroVelTrans;
        Itrans += KiVel * erroVelTrans;
        constrain(Itrans, (float)speedMin, (float)speedMax);
        Dtrans = KdVel * (erroVelTrans - errTrans_ant);
        PidTrans = Ptrans + Itrans + Dtrans;
        errTrans_ant = erroVelTrans;

        Prot = KpRot * erroVelRot;
        Irot += KiRot * erroVelRot;
        constrain(Irot, (float)speedMin, (float)speedMax);
        Drot = KdRot * (erroVelRot - errRot_ant);
        PidRot = Prot + Irot + Drot;
        errRot_ant = erroVelRot;
        // lastRotPid = PidRot;

        // PID output, resta adequar o valor do Pid para ficar dentro do limite do pwm
        PIDTrans->output->setData(constrain((PidTrans) + speedBase, speedMin, speedMax));
        PIDRot->output->setData(PidRot);

        // Calculo de velocidade do motor
        speed->right->setData(
            constrain((int16_t)(PIDTrans->output->getData()) + (int16_t)(PIDRot->output->getData()), speedMin, speedMax));

        speed->left->setData(
            constrain((int16_t)(PIDTrans->output->getData()) - (int16_t)(PIDRot->output->getData()), speedMin, speedMax));

        // Altera a velocidade linear do carrinho
        if (estado == CAR_IN_LINE)
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpointLine");
            setpointPIDTransTarget = PIDTrans->setpointLine->getData();
        }
        else if (estado == CAR_IN_CURVE)
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpointCurve");
            setpointPIDTransTarget = PIDTrans->setpointCurve->getData();
        }

        // if (mapState)
        setpointPIDTransTarget = constrain(((1 - (abs(PIDRot->setpoint->getData() * rotK) / 3500)) * setpointPIDTransTarget), 100, setpointPIDTransTarget);

        // Rampeia a velocidade translacional
        SetpointTransactual = PIDTrans->setpoint->getData();
        if (estado != CAR_STOPPED)
        {
            if (SetpointTransactual <= setpointPIDTransTarget)
            {
                newSetpoint = SetpointTransactual + (accel * ((float)TaskDelay / (float)1000));
                PIDTrans->setpoint->setData(constrain(newSetpoint, SetpointTransactual, setpointPIDTransTarget));
            }
            else
            {
                newSetpoint = SetpointTransactual - (accel * ((float)TaskDelay / (float)1000));
                PIDTrans->setpoint->setData(constrain(newSetpoint, setpointPIDTransTarget, SetpointTransactual));
            }
        }

        // #if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG and !defined GRAPH_DATA
        //         if (iloop > 100)
        //         {
        //             ESP_LOGD(GetName().c_str(), "CarstatusOut: %d | bool : %d", estado, mapState);
        //             ESP_LOGD(GetName().c_str(), "SetPointTrans: %d | Target %d, SetPointRot: %d", PIDTrans->setpoint->getData(), setpointPIDTransTarget, PIDRot->setpoint->getData());
        //             ESP_LOGD(GetName().c_str(), "speedMin: %d | speedMax: %d", speedMin, speedMax);
        //             ESP_LOGD(GetName().c_str(), "PIDRot: %.2f | PIDTrans: %.2f", PIDRot->output->getData(), PIDTrans->output->getData());
        //             ESP_LOGD(GetName().c_str(), "speedLeft: %d | speedRight: %d", speed->left->getData(), speed->right->getData());
        //             ESP_LOGD(GetName().c_str(), "VelTrans: %.2f | VelRot: %.2f\n", VelTrans, VelRot);
        //             iloop = 0;
        //         }
        //         iloop++;
        // #else
        //         if (iloop > 5)
        //         {
        //             ESP_LOGI("", "%d,%d,%d,%d\n", VelTrans, xTaskGetTickCount() * portTICK_PERIOD_MS, PIDTrans->setpoint->getData(), 3500 - robot->getsArray()->getLine()); // VelTrans,time(ms),VelTransSetpoint,LineValue\n
        //             iloop = 0;
        //         }
        //         iloop++;
        // #endif
    }
}