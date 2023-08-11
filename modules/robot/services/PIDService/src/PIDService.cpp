#include "PIDService.hpp"

PIDService::PIDService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();
    this->speed = robot->getSpeed();
    this->status = robot->getStatus();


    // GPIOs dos motores
    //motors.attachMotors(DRIVER_AIN2, DRIVER_AIN1, DRIVER_PWMA, DRIVER_BIN2, DRIVER_BIN1, DRIVER_PWMB);
    motors.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN2, DRIVER_BIN1, DRIVER_PWMB);
    motors.setSTBY(DRIVER_STBY);

    pid_select = status->PID_Select->getData();
    if(!pid_select)
    {
        this->PIDIR = robot->getPIDIR();
        this->PIDTrans = robot->getPIDVel();
        this->PIDRot = robot->getPIDRot();

        this->PIDRot->input->setData(this->robot->getsArray()->getLine());
        PIDTrans->setpoint->setData(0);
        KpVel = PIDTrans->Kp(TUNING)->getData();
        KdVel = PIDTrans->Kd(TUNING)->getData();
        KpRot = PIDRot->Kp(TUNING)->getData();
        KdRot = PIDRot->Kd(TUNING)->getData();
        KpIR = PIDIR->Kp(TUNING)->getData();
        KdIR = PIDIR->Kd(TUNING)->getData();
    }
    else
    {
        this->PIDClassic = robot->getPIDClassic();
        KpIR = PIDClassic->Kp(TUNING)->getData();
        KdIR = PIDClassic->Kd(TUNING)->getData();
    }


    speedTarget = 0;
    fatorCorrecao = speed->CorrectionFactor->getData();


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

        SensorsService::getInstance()->getArraySensors();

        alphaVel = status->alphaVel->getData()/1.0E9;
        alphaRot = status->alphaRot->getData()/1.0E9;
        alphaIR = status->alphaIR->getData()/1.0E9;

        speedBase = speed->base->getData();
        speedMin = speed->min->getData();
        speedMax = speed->max->getData();
        if(!status->FirstMark->getData() && !status->TunningMode->getData())
        {
            accel = speed->initialaccelration->getData();
            speedTarget = speed->initialspeed->getData();
        }
        else if(status->TunningMode->getData())
        {
            accel = speed->initialaccelration->getData();
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
            soma_erroIR = 0;
            soma_erroVelRot = 0;
            soma_erroVelTrans = 0;
            if(!pid_select) PIDTrans->setpoint->setData(0);
            speedTarget = 0;
            speed->CalculatedSpeed->setData(0);
        }

        // Variaveis de calculo para os pids do robô
        if(estado != CAR_STOPPED)
        {
            if (!pid_select)
            {
                KpVel = (PIDTrans->Kp(RealTracklen) != nullptr) ? PIDTrans->Kp(RealTracklen)->getData() : 0;
                KiVel = ((PIDTrans->Ki(RealTracklen) != nullptr) ? PIDTrans->Ki(RealTracklen)->getData() : 0) * TaskDelaySeconds;
                KdVel = ((PIDTrans->Kd(RealTracklen) != nullptr) ? PIDTrans->Kd(RealTracklen)->getData() : 0) / TaskDelaySeconds;

                KpRot = (PIDRot->Kp(RealTracklen) != nullptr) ? PIDRot->Kp(RealTracklen)->getData() : 0;
                KiRot = ((PIDRot->Ki(RealTracklen) != nullptr) ? PIDRot->Ki(RealTracklen)->getData() : 0) * TaskDelaySeconds;
                KdRot = ((PIDRot->Kd(RealTracklen) != nullptr) ? PIDRot->Kd(RealTracklen)->getData() : 0) / TaskDelaySeconds;

                KpIR = (PIDIR->Kp(RealTracklen) != nullptr) ? PIDIR->Kp(RealTracklen)->getData() : 0;
                KdIR = ((PIDIR->Kd(RealTracklen) != nullptr) ? PIDIR->Kd(RealTracklen)->getData() : 0) / TaskDelaySeconds;
            }
            else
            {
                KpIR = (PIDClassic->Kp(RealTracklen) != nullptr) ? PIDClassic->Kp(RealTracklen)->getData() : 0;
                KdIR = ((PIDClassic->Kd(RealTracklen) != nullptr) ? PIDClassic->Kd(RealTracklen)->getData() : 0) / TaskDelaySeconds;   
            }
            
            
        }

        // Velocidade do carrinho
        VelRot = speed->RPMRight_inst->getData() - speed->RPMLeft_inst->getData();   // Rotacional
        VelTrans = speed->RPMRight_inst->getData() + speed->RPMLeft_inst->getData(); // Translacional
        speed->VelTrans->setData(VelTrans);
        speed->VelRot->setData(VelRot);

        IR = robot->getsArray()->getLine(); // posição do robô
        if(!pid_select)
        {
            PIDTrans->input->setData(VelTrans);
            PIDRot->input->setData(VelRot);

            PIDIR->input->setData(IR);

            // Erros atuais
            erroIR = 3500 - IR;
            PIDIR->setpoint->setData(3500);
            PIDIR->erro->setData(erroIR);
            soma_erroIR += erroIR*erroIR;
            PIDIR->erroquad->setData(soma_erroIR);
            erroVelTrans = (float)(PIDTrans->setpoint->getData()) - VelTrans;
            PIDTrans->erro->setData(erroVelTrans);
            soma_erroVelTrans += erroVelTrans*erroVelTrans;
            PIDTrans->erroquad->setData(erroVelTrans);

            // Cálculo do PID para posicionar o robô  na linha
            P_IR = KpIR * erroIR;
            if(PIDIR->UseKdIR->getData())  D_IR = KdIR * (lastIR - IR);
            else D_IR = 0;
            I_IR += KiIR * erroIR;
            PidIR = P_IR + I_IR + D_IR;
            PIDIR->output->setData(PidIR);

            PIDRot->setpoint->setData(PIDIR->output->getData()); // cálculo do setpoint rotacional
            erroVelRot = (float)(PIDRot->setpoint->getData()) - VelRot; //erro rotacional
            PIDRot->erro->setData(erroVelRot);
            soma_erroVelRot += erroVelRot*erroVelRot;
            PIDRot->erroquad->setData(soma_erroVelRot);

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
            soma_erroIR += (erroIR/1000.0)*(erroIR/1000.0);
            PIDClassic->erroquad->setData(soma_erroIR);
            // Cálculo do PID para posicionar o robô  na linha
            P_IR = KpIR * erroIR;
            if(PIDClassic->UseKdIR->getData())  D_IR = KdIR * (lastIR - IR);
            else D_IR = 0;
            I_IR += KiIR * erroIR;
            PidIR = P_IR + I_IR + D_IR;
            PIDClassic->output->setData(PidIR);
            PIDClassic->P_output->setData(P_IR);
            PIDClassic->D_output->setData(D_IR);

            // Calculo de velocidade do motor
            speed->right->setData(
                constrain(speed->CalculatedSpeed->getData() + PIDClassic->output->getData(), speedMin, speedMax));

            speed->left->setData(
                constrain(speed->CalculatedSpeed->getData() - PIDClassic->output->getData(), speedMin, speedMax));

        }

        ControlMotors(speed->left->getData(),speed->right->getData()); // Altera a velocidade dos motores
        // Altera a velocidade linear do carrinho
        if (estado == CAR_IN_LINE && !mapState && status->FirstMark->getData())
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpointLine");
            fatorCorrecao = speed->CorrectionFactorLine->getData();
            switch (TrackLen)
            {
                case XLONG_LINE:
                    speedTarget = speed->XLong_Line->getData(); 
                    break;
                case LONG_LINE:
                    speedTarget = speed->Long_Line->getData(); 
                    break;
                case MEDIUM_LINE:
                    speedTarget = speed->Medium_Line->getData();
                    break;
                case SHORT_LINE:
                    speedTarget = speed->Short_Line->getData();
                    break;
                case SPECIAL_TRACK:
                    speedTarget = speed->Special_Track->getData();
                    fatorCorrecao = speed->CorrectionFactor->getData();
                    break;
                default:
                    speedTarget = speed->Default_speed->getData();
                    break;
            }
            
        }
        else if (estado == CAR_IN_CURVE && !mapState && status->FirstMark->getData())
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpointCurve");
            switch (TrackLen)
            {
                case XLONG_CURVE:
                    speedTarget = speed->XLong_Curve->getData();
                    fatorCorrecao = speed->CorrectionFactorLongCurve->getData();
                    break; 
                case LONG_CURVE:
                    speedTarget = speed->Long_Curve->getData();
                    fatorCorrecao = speed->CorrectionFactorLongCurve->getData();
                    break;
                case MEDIUM_CURVE:
                    speedTarget = speed->Medium_Curve->getData();
                    fatorCorrecao = speed->CorrectionFactorMediumCurve->getData();
                    break;
                case SHORT_CURVE:
                    speedTarget = speed->Short_Curve->getData();
                    fatorCorrecao = speed->CorrectionFactorShortCurve->getData();
                    break;
                case ZIGZAG:
                    speedTarget = speed->ZIGZAG->getData();
                    fatorCorrecao = speed->CorrectionFactorZigZag->getData();
                    break;
                case SPECIAL_TRACK:
                    speedTarget = speed->Special_Track->getData();
                    fatorCorrecao = speed->CorrectionFactor->getData();
                    break;
                default:
                    speedTarget= speed->Default_speed->getData();
                    fatorCorrecao = speed->CorrectionFactor->getData();
                    break;
            }
        }
        else if (mapState && estado != CAR_STOPPED)
        {
            // ESP_LOGD(GetName().c_str(), "Setando setpoint Map");
            speedTarget = speed->SetPointMap->getData();
            fatorCorrecao = speed->CorrectionFactor->getData();
        }
        else if (estado == CAR_TUNING)
        {
            speedTarget = speed->Tunning_speed->getData();
            fatorCorrecao = speed->CorrectionFactor->getData();
        }

        if((mapState || !(status->FirstMark->getData())) && !status->TunningMode->getData()) speedTarget = constrain(((1 - ((float)abs(3500 - robot->getsArray()->getLine()) / 3500.0)) * speedTarget), 0, speedTarget);
        else if(status->CorrectionTrue->getData()) 
        {
            speedTarget = constrain(((1 - (fatorCorrecao*((float)abs(3500 - robot->getsArray()->getLine()) / 3500.0))) * speedTarget), 0, speedTarget);
            //if(abs(3500 - robot->getsArray()->getLine()) > 3000) setpointPIDTransTarget = setpointPIDTransTarget / 2.0 ;
        }  
        // Rampeia a velocidade translacional
        calculatedSpeed = speed->CalculatedSpeed->getData();
        if (estado != CAR_STOPPED)
        {
            if (calculatedSpeed <= speedTarget)
            {
                newSpeed = calculatedSpeed + (accel * TaskDelaySeconds);
                newSpeed = constrain(newSpeed, calculatedSpeed, speedTarget);
                if(!pid_select) PIDTrans->setpoint->setData(newSpeed);
                speed->CalculatedSpeed->setData(newSpeed);
            }
            else
            {
                newSpeed = calculatedSpeed - (desaccel * TaskDelaySeconds);
                newSpeed = constrain(newSpeed, speedTarget, calculatedSpeed);
                if(!pid_select) PIDTrans->setpoint->setData(newSpeed);
                speed->CalculatedSpeed->setData(newSpeed);
            }
        }
        
        // Processo de ajuste dos parametros PID
        // Derivada direcional (Taxa de variacao do sinais)
        double L_trans = 0.0;
        double L_rot = 0.0;
        double L_IR = 0.0;
        if ((PidTrans - lastPIDTrans)!= 0 && status->GD_Optimization->getData() && !pid_select)
        {
            L_trans = (VelTrans - lastVelTrans)/(PidTrans - lastPIDTrans);
        }
        if ((PidRot - lastPIDRot)!= 0 && status->GD_Optimization->getData() && !pid_select)
        {
            L_rot = (VelRot - lastVelRot)/(PidRot - lastPIDRot);
        }
        if ((PidIR - lastPIDIR)!= 0 && status->GD_OptimizationIR->getData())
        {
            L_IR = (IR - lastIR)/(PidIR - lastPIDIR);
        }

        if (estado!=CAR_STOPPED && status->GD_Optimization->getData()  && !pid_select)
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
            KpIR = KpIR + alphaIR*(erroIR*erroIR)*L_IR;
            if(!pid_select)
            {
                PIDIR->Kp(RealTracklen)->setData(KpIR);
                if(PIDIR->UseKdIR->getData()) 
                {
                    KdIR = KdIR + alphaIR*(lastIR - IR)*L_IR*erroIR;
                    PIDIR->Kd(RealTracklen)->setData(KdIR);
                }
            }
            else
            {
                PIDClassic->Kp(RealTracklen)->setData(KpIR);
                if(PIDClassic->UseKdIR->getData()) 
                {
                    KdIR = KdIR + alphaIR*(lastIR - IR)*L_IR*erroIR;
                    PIDClassic->Kd(RealTracklen)->setData(KdIR);
                }

            }
            
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

        if (iloop > 200)
        {
            //ESP_LOGD(GetName().c_str(), "L_trans: %.4f | L_rot : %.4f | L_IR: %.4f", L_trans , L_rot,L_IR);
            if(!pid_select) 
            {
                ESP_LOGD(GetName().c_str(), "SetPointTrans: %.2f | Target %.2f, SetPointRot: %.2f", PIDTrans->setpoint->getData(), speedTarget, PIDRot->setpoint->getData());
                ESP_LOGD(GetName().c_str(), "PIDRot: %.2f | PIDTrans: %.2f", PIDRot->output->getData(), PIDTrans->output->getData());
                ESP_LOGD(GetName().c_str(), "KpVel: %.4f | KpRot: %.4f\n", KpVel, KpRot);
                ESP_LOGD(GetName().c_str(), "KdVel: %.4f | KdRot: %.4f\n", KdVel, KdRot);
            }
            else
            {
                ESP_LOGD(GetName().c_str(), "PIDClassic: %.2f", PIDClassic->output->getData());
            }
            ESP_LOGD(GetName().c_str(), "KdIR: %.4f | KpIR: %.4f\n", KdIR, KpIR);
            ESP_LOGD(GetName().c_str(), "speedLeft: %.2f | speedRight: %.2f", speed->left->getData(), speed->right->getData());
            ESP_LOGD(GetName().c_str(), "VelTrans: %.2f | VelRot: %.2f\n", VelTrans, VelRot);
            ESP_LOGD(GetName().c_str(), "speedMin: %d | speedMax: %d | speedBase: %d", speedMin, speedMax, speedBase);
            iloop = 0;
        }
        iloop++;

    }
}

void PIDService::ControlMotors(float left, float right)
{
    CarState state = (CarState)status->robotState->getData();

    // if (iloop >= 200 && !status->robotIsMapping->getData())
    // {
    //     iloop = 0;
    //     ESP_LOGD("MotorsService", "State: %d", state);
    // }
    // iloop++;

    if (state != CAR_STOPPED) // verificar se o carrinho deveria se mover
    {
      // motors.motorForward(0);                        // motor 0 ligado para frente
      // motors.motorForward(1);                        // motor 1 ligado para frente
      motors.motorSpeed(0, left);  // velocidade do motor 0
      motors.motorSpeed(1, right); // velocidade do motor 1
    }
    else
    {
      motors.motorsStop();
    }
}