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

    updatePID(DEFAULT, CAR_STOPPED);

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
            P = 0;
            D = 0;
            soma_erro = 0;
            speedTarget = 0;
            speed->CalculatedSpeed->setData(0);
        }

        // Variaveis de calculo para os pids do robô
        if (estado != CAR_STOPPED)
            updatePID(RealTracklen, estado);

        // Velocidade do carrinho
        VelRot = speed->RPMRight_inst->getData() - speed->RPMLeft_inst->getData();   // Rotacional
        VelTrans = speed->RPMRight_inst->getData() + speed->RPMLeft_inst->getData(); // Translacional
        speed->VelTrans->setData(VelTrans);
        speed->VelRot->setData(VelRot);

        SensorArrayPosition = robot->getsArray()->getLine(); // posição do robô
        erro = 3500 - SensorArrayPosition;
        dataPID->setpoint->setData(3500);
        dataPID->erro->setData(erro);
        soma_erro += (erro / 1000.0) * (erro / 1000.0);
        dataPID->erroquad->setData(soma_erro);

        // Cálculo do PID para posicionar o robô  na linha
        float pid = calculatePID();
        
        dataPID->output->setData(pid)
        dataPID->P_output->setData(P);
        dataPID->D_output->setData(D);

        // Armazenamento dos parametros de controle atuais
        lastSensorArrayPosition = SensorArrayPosition;

        // Calculo de velocidade do motor
        speed->right->setData(
            constrain(speed->CalculatedSpeed->getData() + dataPID->output->getData(), speedMin, speedMax));

        speed->left->setData(
            constrain(speed->CalculatedSpeed->getData() - dataPID->output->getData(), speedMin, speedMax));

        ControlMotors(speed->left->getData(), speed->right->getData()); // Altera a velocidade dos motores

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

        if (iloop > 200)
        {
            ESP_LOGD(GetName().c_str(), "dataPID: %.2f", dataPID->output->getData());
            ESP_LOGD(GetName().c_str(), "Kd: %.4f | Kp: %.4f\n", Kd, Kp);
            ESP_LOGD(GetName().c_str(), "speedLeft: %.2f | speedRight: %.2f", speed->left->getData(), speed->right->getData());
            ESP_LOGD(GetName().c_str(), "VelTrans: %.2f | VelRot: %.2f\n", VelTrans, VelRot);
            ESP_LOGD(GetName().c_str(), "speedMin: %d | speedMax: %d | speedBase: %d", speedMin, speedMax, speedBase);
            iloop = 0;
        }
        iloop++;
    }
}

// Rotina que é executada com base no acionamento do timer
bool IRAM_ATTR PIDService::timer_group_isr_callback(void *args)
{
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(SemaphoreTimer, &high_task_awoken);
    return (high_task_awoken == pdTRUE);
}

void updatePID(TrackSegment segment, CarState carState)
{
    PID pid = getTrackSegmentPID(segment, carState);

    Kp = pid.KP;
    Kd = pid.KD / TaskDelaySeconds;
    Ki = pid.KI;
}

float calculatePID()
{
    P = Kp * erro;
    D = Kd * (lastSensorArrayPosition - SensorArrayPosition);
    I += Ki * erro;
    
    return P + D + I;
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
    speed->CalculatedSpeed->setData(newSpeed);
}
