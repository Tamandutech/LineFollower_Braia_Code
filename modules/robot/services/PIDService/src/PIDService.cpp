#include "PIDService.hpp"

PIDService::PIDService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();;
    this->speed = robot->getSpeed();
    this->status = robot->getStatus();
    this->PIDTrans = robot->getPIDVel();
    this->PIDRot = robot->getPIDRot();

    this->PIDRot->input->setData(this->robot->getsArray()->getLine());
    PIDTrans->setpoint->setData(0);
    setpointPIDTransTarget = 0;
    rotK = PIDRot->Krot->getData();
    KpVel = PIDTrans->Kp(CAR_TUNING)->getData();
    KdVel = PIDTrans->Kd(CAR_TUNING)->getData();
    KpRot = PIDRot->Kp(CAR_TUNING)->getData();
    KdRot = PIDRot->Kd(CAR_TUNING)->getData();

};

void PIDService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);

        estado = (CarState)status->robotState->getData();
        TrackLen = (TrackState)status->TrackStatus->getData();
        mapState = status->robotIsMapping->getData();

        speedBase = speed->base->getData();
        speedMin = speed->min->getData();
        speedMax = speed->max->getData();
        if(!status->FirstMark->getData() && !status->TunningMode->getData())
        {
            accel = speed->initialaccelration->getData();
            setpointPIDTransTarget = speed->initialspeed->getData();
        }
        else
        {
            accel = speed->accelration->getData();
        }
        desaccel = speed->desaccelration->getData();
        

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
        if(estado != CAR_STOPPED)
        {
            KpVel = (PIDTrans->Kp(estado) != nullptr) ? PIDTrans->Kp(estado)->getData() : 0;
            KiVel = (PIDTrans->Ki(estado) != nullptr) ? PIDTrans->Ki(estado)->getData() : 0;
            KdVel = (PIDTrans->Kd(estado) != nullptr) ? PIDTrans->Kd(estado)->getData() : 0;

            KpRot = (PIDRot->Kp(estado) != nullptr) ? PIDRot->Kp(estado)->getData() : 0;
            KiRot = (PIDRot->Ki(estado) != nullptr) ? PIDRot->Ki(estado)->getData() : 0;
            KdRot = (PIDRot->Kd(estado) != nullptr) ? PIDRot->Kd(estado)->getData() : 0;
        }
        // Velocidade do carrinho
        VelRot = speed->RPMRight_inst->getData() - speed->RPMLeft_inst->getData();   // Rotacional
        VelTrans = speed->RPMRight_inst->getData() + speed->RPMLeft_inst->getData(); // Translacional

        // Erros atuais
        //  rotK (porcentagem do erro do PID rotcioanl que representará a variação máxima de RPM dos motores)
        PIDRot->setpoint->setData((3500 - robot->getsArray()->getLine()) / rotK); // cálculo do setpoint rotacional
        erroVelTrans = (float)(PIDTrans->setpoint->getData()) - VelTrans;
        if (iloop > 100)
        {
            ESP_LOGD(GetName().c_str(), "PIDTrans->setpoint: %d", PIDTrans->setpoint->getData());
            iloop = 0;
        }
        iloop++;
        erroVelRot = (float)(PIDRot->setpoint->getData()) - VelRot;

        // calculando Pids rotacional e translacional
        Ptrans = KpVel * erroVelTrans;
        Itrans += KiVel * erroVelTrans;
        constrain(Itrans, (float)speedMin, (float)speedMax);
        //Dtrans = KdVel * (erroVelTrans - errTrans_ant);
        Dtrans = KdVel * (lastVelTrans - VelTrans);
        PidTrans = Ptrans + Itrans + Dtrans;
        errTrans_ant = erroVelTrans;
        //lastVelTrans = VelTrans;

        Prot = KpRot * erroVelRot;
        Irot += KiRot * erroVelRot;
        constrain(Irot, (float)speedMin, (float)speedMax);
        //Drot = KdRot * (erroVelRot - errRot_ant);
        Drot = KdRot * (lastVelRot - VelRot);
        PidRot = Prot + Irot + Drot;
        errRot_ant = erroVelRot;
        //lastVelRot = VelRot;


        // PID output, resta adequar o valor do Pid para ficar dentro do limite do pwm
        PIDTrans->output->setData(constrain((PidTrans) + speedBase, speedMin, speedMax));
        PIDRot->output->setData(PidRot);
        
        lastPIDTrans = PidTrans;
        lastPIDRot = PidRot;

        // Calculo de velocidade do motor
        speed->right->setData(
            constrain(PIDTrans->output->getData() + PIDRot->output->getData(), speedMin, speedMax));

        speed->left->setData(
            constrain(PIDTrans->output->getData() - PIDRot->output->getData(), speedMin, speedMax));

        // Altera a velocidade linear do carrinho
        if (estado == CAR_IN_LINE && !mapState && status->FirstMark->getData())
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpointLine");
            rotK = PIDRot->Krot->getData();
            switch (TrackLen)
            {
                case LONG_LINE:
                    setpointPIDTransTarget = speed->Long_Line->getData(); 
                    break;
                case MEDIUM_LINE:
                    setpointPIDTransTarget = speed->Medium_Line->getData();
                    break;
                case SHORT_LINE:
                    setpointPIDTransTarget = speed->Short_Line->getData();
                    break;
                default:
                    setpointPIDTransTarget = speed->initialspeed->getData();
                    break;
            }
            
        }
        else if (estado == CAR_IN_CURVE && !mapState && status->FirstMark->getData())
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpointCurve");
            switch (TrackLen)
            {
                case LONG_CURVE:
                    setpointPIDTransTarget = speed->Long_Curve->getData();
                    rotK = PIDRot->KrotLongCurve->getData();
                    break;
                case MEDIUM_CURVE:
                    setpointPIDTransTarget = speed->Medium_Curve->getData();
                    rotK = PIDRot->KrotMediumCurve->getData();
                    break;
                case SHORT_CURVE:
                    setpointPIDTransTarget = speed->Short_Curve->getData();
                    rotK = PIDRot->KrotShortCurve->getData();
                    break;
                default:
                    setpointPIDTransTarget = speed->initialspeed->getData();
                    rotK = PIDRot->Krot->getData();
                    break;
            }
        }
        else if (mapState && estado != CAR_STOPPED)
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpoint Map");
            setpointPIDTransTarget = speed->SetPointMap->getData();
            rotK = PIDRot->Krot->getData();
        }
        else if (estado == CAR_TUNING)
        {
            setpointPIDTransTarget = speed->Tunning_speed->getData();
            rotK = PIDRot->Krot->getData();
        }

        if((mapState || !(status->FirstMark->getData())) && !status->TunningMode->getData()) setpointPIDTransTarget = constrain(((1 - ((float)abs(3500 - robot->getsArray()->getLine()) / 3500.0)) * setpointPIDTransTarget), 0, setpointPIDTransTarget);
        else if(status->CorrectionTrue->getData()) 
        {
            setpointPIDTransTarget = constrain(((1 - (PIDTrans->CorrectionFactor->getData()*((float)abs(3500 - robot->getsArray()->getLine()) / 3500.0))) * setpointPIDTransTarget), 0, setpointPIDTransTarget);
            //if(abs(3500 - robot->getsArray()->getLine()) > 3000) setpointPIDTransTarget = setpointPIDTransTarget / 2.0 ;
        }  
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
                newSetpoint = SetpointTransactual - (desaccel * ((float)TaskDelay / (float)1000));
                PIDTrans->setpoint->setData(constrain(newSetpoint, setpointPIDTransTarget, SetpointTransactual));
            }
        }
        
        // Processo de ajuste dos parametros PID
        // Derivada direcional (Taxa de variacao do sinais)
        float L_trans = 0.0;
        float L_rot = 0.0;
        if ((PidTrans - lastPIDTrans)!= 0)
        {
            L_trans = (VelTrans - lastVelTrans)/(PidTrans - lastPIDTrans);
        }
        if ((PidRot - lastPIDRot)!= 0)
        {
            L_rot = (VelRot - lastVelRot)/(PidRot - lastPIDRot);
        }

        if (status->TunningMode->getData() && estado!=CAR_STOPPED)
        {   

            KpVel = KpVel + alpha*(erroVelTrans*erroVelTrans)*L_trans;
            KdVel = KdVel + alpha*(lastVelTrans - VelTrans)*L_trans*Dtrans;
        
            KpRot = KpRot + alpha*(erroVelRot*erroVelRot)*L_rot;
            KdRot = KdRot + alpha*(lastVelRot - VelRot)*L_rot*Drot;

            // Alterar os parametros do controle PID rot e trans
            PIDTrans->Kp(estado)->setData(KpVel);
            PIDTrans->Kd(estado)->setData(KdVel);
            PIDRot->Kp(estado)->setData(KpRot);
            PIDRot->Kd(estado)->setData(KdRot);

        }
        // Armazenamento dos parametros de controle atuais 
        lastVelTrans = VelTrans;
        lastVelRot = VelRot;
        lastPIDTrans = PidTrans;
        lastPIDRot = PidRot;

// #if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG and !defined GRAPH_DATA
//         if (iloop > 30)
//         {
        if (iloop > 100)
        {
            ESP_LOGD(GetName().c_str(), "L_trans: %.2f | L_rot : %.2f", L_trans , L_rot);
            ESP_LOGD(GetName().c_str(), "SetPointTrans: %d | Target %d, SetPointRot: %d", PIDTrans->setpoint->getData(), setpointPIDTransTarget, PIDRot->setpoint->getData());
//             ESP_LOGD(GetName().c_str(), "speedMin: %d | speedMax: %d | speedBase: %d", speedMin, speedMax, speedBase);
//             ESP_LOGD(GetName().c_str(), "PIDRot: %.2f | PIDTrans: %.2f", PIDRot->output->getData(), PIDTrans->output->getData());
//             ESP_LOGD(GetName().c_str(), "speedLeft: %d | speedRight: %d", speed->left->getData(), speed->right->getData());
//             ESP_LOGD(GetName().c_str(), "VelTrans: %.2f | VelRot: %.2f\n", VelTrans, VelRot);
            iloop = 0;
        }
        iloop++;

    }
}