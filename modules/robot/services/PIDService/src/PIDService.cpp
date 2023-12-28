#include "PIDService.hpp"

SemaphoreHandle_t PIDService::SemaphoreTimer;
PIDService::PIDService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();
    this->speed = robot->getSpeed();
    this->status = robot->getStatus();

    // GPIOs dos motores
    // motors.attachMotors(DRIVER_AIN2, DRIVER_AIN1, DRIVER_PWMA, DRIVER_BIN2, DRIVER_BIN1, DRIVER_PWMB);
    motors.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN2, DRIVER_BIN1, DRIVER_PWMB);
    motors.setSTBY(DRIVER_STBY);

    pid_select = status->PID_Select->getData();

    this->PIDClassic = robot->getPIDClassic();
    KpIR = PIDClassic->Kp(TUNING)->getData();
    KdIR = PIDClassic->Kd(TUNING)->getData() / TaskDelaySeconds;

    speedTarget = 0;

    // Inicializa o semáforo
    SemaphoreTimer = xSemaphoreCreateBinary();

    // Configura o timer
    timer_config_t config;
    config.alarm_en = TIMER_ALARM_EN;
    config.counter_en = TIMER_PAUSE;
    config.counter_dir = TIMER_COUNT_UP;
    config.auto_reload = TIMER_AUTORELOAD_EN; // define se o contador do timer deve reiniciar automaticamente
    config.divider = 16;                      // Define o valor pelo qual o clock deve ser dividido

    timer_init(TIMER_GROUP_0, TIMER_0, &config);                                             // inicializa timer
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);                                      // zera o contador do timer
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TaskDelaySeconds * (TIMER_BASE_CLK / 16)); // define até quando o timer deve contar
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);                                               // ativa interrupção
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, timer_group_isr_callback, NULL, 0);       // define a função executada quando ocorre interrupção
    timer_start(TIMER_GROUP_0, TIMER_0);
}

void PIDService::Run()
{
    for (;;)
    {
        // Trava a task até o semáfaro ser liberado com base no timer
        xSemaphoreTake(SemaphoreTimer, portMAX_DELAY);
        estado = (CarState)status->robotState->getData();

        RealTracklen = (TrackState)status->RealTrackStatus->getData();
        mapState = status->robotIsMapping->getData();

        SensorsService::getInstance()->getArraySensors();

        alphaVel = status->alphaVel->getData() / 1.0E9;
        alphaRot = status->alphaRot->getData() / 1.0E9;
        alphaIR = status->alphaIR->getData() / 1.0E9;

        speedBase = speed->base->getData();
        speedMin = speed->min->getData();
        speedMax = speed->max->getData();
        if (!status->FirstMark->getData() && !status->TunningMode->getData())
        {
            accel = speed->initialaccelration->getData();
            speedTarget = speed->initialspeed->getData();
        }
        else if (status->TunningMode->getData())
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
            if (!pid_select)
                PIDTrans->setpoint->setData(0);
            speedTarget = 0;
            speed->CalculatedSpeed->setData(0);
        }

        // Variaveis de calculo para os pids do robô
        if (estado != CAR_STOPPED)
        {
            
        
                KpIR = (PIDClassic->Kp(RealTracklen) != nullptr) ? PIDClassic->Kp(RealTracklen)->getData() : 0;
                KdIR = ((PIDClassic->Kd(RealTracklen) != nullptr) ? PIDClassic->Kd(RealTracklen)->getData() : 0) / TaskDelaySeconds;
            
        }

        // Velocidade do carrinho
        VelRot = speed->RPMRight_inst->getData() - speed->RPMLeft_inst->getData();   // Rotacional
        VelTrans = speed->RPMRight_inst->getData() + speed->RPMLeft_inst->getData(); // Translacional
        speed->VelTrans->setData(VelTrans);
        speed->VelRot->setData(VelRot);

        IR = robot->getsArray()->getLine(); // posição do robô
        if (!pid_select)
        {
            PIDTrans->input->setData(VelTrans);
            PIDRot->input->setData(VelRot);

            PIDIR->input->setData(IR);

            // Erros atuais
            erroIR = 3500 - IR;
            PIDIR->setpoint->setData(3500);
            PIDIR->erro->setData(erroIR);
            soma_erroIR += erroIR * erroIR;
            PIDIR->erroquad->setData(soma_erroIR);
            erroVelTrans = (float)(PIDTrans->setpoint->getData()) - VelTrans;
            PIDTrans->erro->setData(erroVelTrans);
            soma_erroVelTrans += erroVelTrans * erroVelTrans;
            PIDTrans->erroquad->setData(erroVelTrans);

            // Cálculo do PID para posicionar o robô  na linha
            P_IR = KpIR * erroIR;
            if (PIDIR->UseKdIR->getData())
                D_IR = KdIR * (lastIR - IR);
            else
                D_IR = 0;
            I_IR += KiIR * erroIR;
            PidIR = P_IR + I_IR + D_IR;
            PIDIR->output->setData(PidIR);

            PIDRot->setpoint->setData(PIDIR->output->getData());        // cálculo do setpoint rotacional
            erroVelRot = (float)(PIDRot->setpoint->getData()) - VelRot; // erro rotacional
            PIDRot->erro->setData(erroVelRot);
            soma_erroVelRot += erroVelRot * erroVelRot;
            PIDRot->erroquad->setData(soma_erroVelRot);

            // calculando Pids rotacional e translacional
            Ptrans = KpVel * erroVelTrans;
            Itrans += KiVel * erroVelTrans;
            constrain(Itrans, (float)speedMin, (float)speedMax);
            // Dtrans = KdVel * (erroVelTrans - errTrans_ant);
            Dtrans = KdVel * (lastVelTrans - VelTrans);
            PidTrans = Ptrans + Itrans + Dtrans;
            // errTrans_ant = erroVelTrans;
            // lastVelTrans = VelTrans;

            Prot = KpRot * erroVelRot;
            Irot += KiRot * erroVelRot;
            constrain(Irot, (float)speedMin, (float)speedMax);
            // Drot = KdRot * (erroVelRot - errRot_ant);
            Drot = KdRot * (lastVelRot - VelRot);
            PidRot = Prot + Irot + Drot;
            // errRot_ant = erroVelRot;
            // lastVelRot = VelRot;

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
            soma_erroIR += (erroIR / 1000.0) * (erroIR / 1000.0);
            PIDClassic->erroquad->setData(soma_erroIR);
            // Cálculo do PID para posicionar o robô  na linha
            P_IR = KpIR * erroIR;
            if (PIDClassic->UseKdIR->getData())
                D_IR = KdIR * (lastIR - IR);
            else
                D_IR = 0;
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

        ControlMotors(speed->left->getData(), speed->right->getData()); // Altera a velocidade dos motores

        /* PEDRO */

        // Altera a velocidade linear do carrinho
        if (!mapState && status->FirstMark->getData())
        {
            TrackState trackState = (TrackState)status->TrackStatus->getData();
            selectTracktState(trackState);
        }

        else if (mapState && estado != CAR_STOPPED)
        {
            speedTarget = speed->SetPointMap->getData();
        }
        else if (estado == CAR_TUNING)
        {
            speedTarget = speed->Tunning_speed->getData();
        }

        // Rampeia a velocidade translacional

        if (estado != CAR_STOPPED)
        {
            float speedValue = speed->CalculatedSpeed->getData();
            float newSpeed = calculateSpeed(accel, speedValue);

            if (speedValue >= speedTarget)
                newSpeed = calculateSpeed(-desaccel, speedValue);

            storingSpeedValue(newSpeed);
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
            if (!pid_select)
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

    if (state != CAR_STOPPED)
    {
        motors.motorSpeed(0, left);  // velocidade do motor 0
        motors.motorSpeed(1, right); // velocidade do motor 1
    }
    else
    {
        motors.motorsStop();
    }
}

float PIDService::calculateSpeed(float acceleration, float speedValue)
{
    float newSpeed = speedValue + (acceleration * TaskDelaySeconds);
    return constrain(newSpeed, speedValue, speedTarget);
}

void PIDService::storingSpeedValue(float newSpeed)
{
    if (!pid_select)
        PIDTrans->setpoint->setData(newSpeed);
    speed->CalculatedSpeed->setData(newSpeed);
}

// Rotina que é executada com base no acionamento do timer
bool IRAM_ATTR PIDService::timer_group_isr_callback(void *args)
{
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(SemaphoreTimer, &high_task_awoken);
    return (high_task_awoken == pdTRUE);
}
