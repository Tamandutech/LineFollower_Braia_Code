#include "PIDService.hpp"

PIDService::PIDService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();;
    this->speed = robot->getSpeed();
    this->status = robot->getStatus();

    this->PIDTrans = robot->getPIDVel();
    this->PIDRot = robot->getPIDRot();
    this->PIDIR = robot->getPIDIR();

    this->PIDRot->input->setData(this->robot->getsArray()->getLine());
    PIDTrans->setpoint->setData(0);
    setpointPIDTransTarget = 0;
    KpVel = PIDTrans->Kp(TUNING)->getData();
    KdVel = PIDTrans->Kd(TUNING)->getData();
    KpRot = PIDRot->Kp(TUNING)->getData();
    KdRot = PIDRot->Kd(TUNING)->getData();
    KpIR = PIDIR->Kp(TUNING)->getData();
    KdIR = PIDIR->Kd(TUNING)->getData();
    fatorCorrecao = PIDTrans->CorrectionFactor->getData();

    this->PIDClassic = robot->getPIDClassic();

};

void PIDService::Run()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);

        estado = (CarState)status->robotState->getData();
        TrackLen = (TrackState)status->TrackStatus->getData();
        RealTracklen = (TrackState)status->RealTrackStatus->getData();
        mapState = status->robotIsMapping->getData();

        alphaVel = status->alphaVel->getData()/1.0E9;
        alphaRot = status->alphaRot->getData()/1.0E9;
        alphaIR = status->alphaIR->getData()/1.0E9;

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
            P_IR = 0;
            D_IR = 0;
            Ptrans = 0;
            Dtrans = 0;
            Itrans = 0;
            Prot = 0;
            Drot = 0;
            Irot = 0;
            PIDTrans->setpoint->setData(0);
            setpointPIDTransTarget = 0;
        }

        // Variaveis de calculo para os pids do robô
        if(estado != CAR_STOPPED)
        {
            if (!status->PID_Select->getData())
            {
                KpVel = (PIDTrans->Kp(RealTracklen) != nullptr) ? PIDTrans->Kp(RealTracklen)->getData() : 0;
                KiVel = (PIDTrans->Ki(RealTracklen) != nullptr) ? PIDTrans->Ki(RealTracklen)->getData() : 0;
                KdVel = (PIDTrans->Kd(RealTracklen) != nullptr) ? PIDTrans->Kd(RealTracklen)->getData() : 0;

                KpRot = (PIDRot->Kp(RealTracklen) != nullptr) ? PIDRot->Kp(RealTracklen)->getData() : 0;
                KiRot = (PIDRot->Ki(RealTracklen) != nullptr) ? PIDRot->Ki(RealTracklen)->getData() : 0;
                KdRot = (PIDRot->Kd(RealTracklen) != nullptr) ? PIDRot->Kd(RealTracklen)->getData() : 0;

                KpIR = (PIDIR->Kp(RealTracklen) != nullptr) ? PIDIR->Kp(RealTracklen)->getData() : 0;
                KdIR = (PIDIR->Kd(RealTracklen) != nullptr) ? PIDIR->Kd(RealTracklen)->getData() : 0;   
            }
            else
            {
                KpIR = (PIDClassic->Kp(RealTracklen) != nullptr) ? PIDClassic->Kp(RealTracklen)->getData() : 0;
                KdIR = (PIDClassic->Kd(RealTracklen) != nullptr) ? PIDClassic->Kd(RealTracklen)->getData() : 0;   
            }
            
            
        }

        // Velocidade do carrinho
        VelRot = speed->RPMRight_inst->getData() - speed->RPMLeft_inst->getData();   // Rotacional
        VelTrans = speed->RPMRight_inst->getData() + speed->RPMLeft_inst->getData(); // Translacional

        IR = robot->getsArray()->getLine(); // posição do robô
        if(!status->PID_Select->getData())
        {
            PIDTrans->input->setData(VelTrans);
            PIDRot->input->setData(VelRot);

            PIDIR->input->setData(IR);
            speed->VelTrans->setData(VelTrans);
            speed->VelRot->setData(VelRot);

            // Erros atuais
            erroIR = 3500 - IR;
            PIDIR->setpoint->setData(3500);
            PIDIR->erro->setData(erroIR);
            erroVelTrans = (float)(PIDTrans->setpoint->getData()) - VelTrans;
            PIDTrans->erro->setData(erroVelTrans);

            // Cálculo do PID para posicionar o robô  na linha
            P_IR = KpIR * erroIR;
            if(PIDIR->UseKdIR->getData())  D_IR = KdIR * (lastIR - IR);
            else D_IR = 0;
            PidIR = P_IR + D_IR;
            PIDIR->output->setData(PidIR);

            PIDRot->setpoint->setData(PIDIR->output->getData()); // cálculo do setpoint rotacional
            erroVelRot = (float)(PIDRot->setpoint->getData()) - VelRot; //erro rotacional
            PIDRot->erro->setData(erroVelRot);

            // calculando Pids rotacional e translacional
            Ptrans = KpVel * erroVelTrans;
            Itrans += KiVel * erroVelTrans;
            constrain(Itrans, (float)speedMin, (float)speedMax);
            //Dtrans = KdVel * (erroVelTrans - errTrans_ant);
            Dtrans = KdVel * (lastVelTrans - VelTrans);
            PidTrans = Ptrans + Itrans + Dtrans;
            //errTrans_ant = erroVelTrans;
            //lastVelTrans = VelTrans;

            Prot = KpRot * erroVelRot;
            Irot += KiRot * erroVelRot;
            constrain(Irot, (float)speedMin, (float)speedMax);
            //Drot = KdRot * (erroVelRot - errRot_ant);
            Drot = KdRot * (lastVelRot - VelRot);
            PidRot = Prot + Irot + Drot;
            //errRot_ant = erroVelRot;
            //lastVelRot = VelRot;


            // PID output, resta adequar o valor do Pid para ficar dentro do limite do pwm
            PIDTrans->output->setData(constrain((PidTrans) + speedBase, speedMin, speedMax));
            PIDRot->output->setData(PidRot);


            // Calculo de velocidade do motor
            speed->right->setData(
                constrain(PIDTrans->output->getData() + PIDRot->output->getData(), speedMin, speedMax));

            speed->left->setData(
                constrain(PIDTrans->output->getData() - PIDRot->output->getData(), speedMin, speedMax));
        }
        else
        {

            erroIR = 3500 - IR;
            PIDClassic->setpoint->setData(3500);
            PIDClassic->erro->setData(erroIR);
            // Cálculo do PID para posicionar o robô  na linha
            P_IR = KpIR * erroIR;
            if(PIDClassic->UseKdIR->getData())  D_IR = KdIR * (lastIR - IR);
            else D_IR = 0;
            PidIR = P_IR + D_IR;
            PIDClassic->output->setData(PidIR);

            // Calculo de velocidade do motor
            speed->right->setData(
                constrain(setpointPIDTransTarget + PIDClassic->output->getData(), speedMin, speedMax));

            speed->left->setData(
                constrain(setpointPIDTransTarget - PIDClassic->output->getData(), speedMin, speedMax));

        }

        // Altera a velocidade linear do carrinho
        if (estado == CAR_IN_LINE && !mapState && status->FirstMark->getData())
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpointLine");
            fatorCorrecao = PIDTrans->CorrectionFactorLine->getData();
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
                case SPECIAL_TRACK:
                    setpointPIDTransTarget = speed->Special_Track->getData();
                    fatorCorrecao = PIDTrans->CorrectionFactor->getData();
                    break;
                default:
                    setpointPIDTransTarget = speed->Default_speed->getData();
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
                    fatorCorrecao = PIDTrans->CorrectionFactorLongCurve->getData();
                    break;
                case MEDIUM_CURVE:
                    setpointPIDTransTarget = speed->Medium_Curve->getData();
                    fatorCorrecao = PIDTrans->CorrectionFactorMediumCurve->getData();
                    break;
                case SHORT_CURVE:
                    setpointPIDTransTarget = speed->Short_Curve->getData();
                    fatorCorrecao = PIDTrans->CorrectionFactorShortCurve->getData();
                    break;
                case ZIGZAG:
                    setpointPIDTransTarget = speed->ZIGZAG->getData();
                    fatorCorrecao = PIDTrans->CorrectionFactorZigZag->getData();
                    break;
                case SPECIAL_TRACK:
                    setpointPIDTransTarget = speed->Special_Track->getData();
                    fatorCorrecao = PIDTrans->CorrectionFactor->getData();
                    break;
                default:
                    setpointPIDTransTarget = speed->Default_speed->getData();
                    fatorCorrecao = PIDTrans->CorrectionFactor->getData();
                    break;
            }
        }
        else if (mapState && estado != CAR_STOPPED)
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpoint Map");
            setpointPIDTransTarget = speed->SetPointMap->getData();
            fatorCorrecao = PIDTrans->CorrectionFactor->getData();
        }
        else if (estado == CAR_TUNING)
        {
            setpointPIDTransTarget = speed->Tunning_speed->getData();
            fatorCorrecao = PIDTrans->CorrectionFactor->getData();
        }

        if((mapState || !(status->FirstMark->getData())) && !status->TunningMode->getData()) setpointPIDTransTarget = constrain(((1 - ((float)abs(3500 - robot->getsArray()->getLine()) / 3500.0)) * setpointPIDTransTarget), 0, setpointPIDTransTarget);
        else if(status->CorrectionTrue->getData()) 
        {
            setpointPIDTransTarget = constrain(((1 - (fatorCorrecao*((float)abs(3500 - robot->getsArray()->getLine()) / 3500.0))) * setpointPIDTransTarget), 0, setpointPIDTransTarget);
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
        double L_trans = 0.0;
        double L_rot = 0.0;
        double L_IR = 0.0;
        if ((PidTrans - lastPIDTrans)!= 0 && status->GD_Optimization->getData())
        {
            L_trans = (VelTrans - lastVelTrans)/(PidTrans - lastPIDTrans);
        }
        if ((PidRot - lastPIDRot)!= 0 && status->GD_Optimization->getData())
        {
            L_rot = (VelRot - lastVelRot)/(PidRot - lastPIDRot);
        }
        if ((PidIR - lastPIDIR)!= 0 && status->GD_OptimizationIR->getData())
        {
            L_IR = (IR - lastIR)/(PidIR - lastPIDIR);
        }

        if (estado!=CAR_STOPPED && status->GD_Optimization->getData())
        {   

            KpVel = KpVel + alphaVel*(erroVelTrans*erroVelTrans)*L_trans;
            KdVel = KdVel + alphaVel*(lastVelTrans - VelTrans)*L_trans*erroVelTrans;
            if(PIDTrans->UseKiVel->getData()) 
            {
                KiVel = KiVel + alphaVel*(Itrans / KiVel)*L_trans*erroVelTrans;
                PIDTrans->Ki(RealTracklen)->setData(KiVel);
            }
        
            KpRot = KpRot + alphaRot*(erroVelRot*erroVelRot)*L_rot;
            KdRot = KdRot + alphaRot*(lastVelRot - VelRot)*L_rot*erroVelRot;

            // Alterar os parametros do controle PID rot e trans
            PIDTrans->Kp(RealTracklen)->setData(KpVel);
            PIDTrans->Kd(RealTracklen)->setData(KdVel);
            PIDRot->Kp(RealTracklen)->setData(KpRot);
            PIDRot->Kd(RealTracklen)->setData(KdRot);

        }
        if(estado != CAR_STOPPED && status->GD_OptimizationIR->getData())
        {
            if(PIDIR->UseKdIR->getData()) 
            {
                KdIR = KdIR + alphaIR*(lastIR - IR)*L_IR*erroIR;
                PIDIR->Kd(RealTracklen)->setData(KdIR);
            }
            KpIR = KpIR + alphaIR*(erroIR*erroIR)*L_IR;
            PIDIR->Kp(RealTracklen)->setData(KpIR);
        }
        // Armazenamento dos parametros de controle atuais 
        lastVelTrans = VelTrans;
        lastVelRot = VelRot;
        lastIR = IR;
        lastPIDTrans = PidTrans;
        lastPIDRot = PidRot;
        lastPIDIR = PidIR;
        errTrans_ant = erroVelTrans;
        errRot_ant = erroVelRot;

        if (iloop > 100)
        {
            //ESP_LOGD(GetName().c_str(), "L_trans: %.4f | L_rot : %.4f | L_IR: %.4f", L_trans , L_rot,L_IR);
            ESP_LOGD(GetName().c_str(), "SetPointTrans: %.2f | Target %d, SetPointRot: %.2f", PIDTrans->setpoint->getData(), setpointPIDTransTarget, PIDRot->setpoint->getData());
//             ESP_LOGD(GetName().c_str(), "speedMin: %d | speedMax: %d | speedBase: %d", speedMin, speedMax, speedBase);
            ESP_LOGD(GetName().c_str(), "PIDRot: %.2f | PIDTrans: %.2f", PIDRot->output->getData(), PIDTrans->output->getData());
            ESP_LOGD(GetName().c_str(), "speedLeft: %.2f | speedRight: %.2f", speed->left->getData(), speed->right->getData());
            ESP_LOGD(GetName().c_str(), "VelTrans: %.2f | VelRot: %.2f\n", VelTrans, VelRot);
            //ESP_LOGD(GetName().c_str(), "KpVel: %.4f | KpRot: %.4f\n", KpVel, KpRot);
            //ESP_LOGD(GetName().c_str(), "KdVel: %.4f | KdRot: %.4f\n", KdVel, KdRot);
            //ESP_LOGD(GetName().c_str(), "KdIR: %.4f | KpIR: %.4f\n", KdIR, KpIR);
            iloop = 0;
        }
        iloop++;

    }
}