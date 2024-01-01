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

    // Inicializa o semáforo
    SemaphoreTimer = xSemaphoreCreateBinary();

    //Inicializa o timer responsável por controlar a periodicidade da Task
    TimerInit(TaskDelaySeconds);

}

void PIDService::Run()
{
    for (;;)
    {
        // Trava a task até o semáfaro ser liberado com base no timer
        xSemaphoreTake(SemaphoreTimer, portMAX_DELAY);
        estado = (CarState)status->robotState->getData();
        if(estado == CAR_STOPPED)
        {
            resetGlobalVariables();
            motors.motorsStop();
        }
        else
        {
            RealTracklen = (TrackSegment)status->RealTrackStatus->getData();

            // Velocidade do carrinho
            float VelRot = (speed->RPMRight_inst->getData() - speed->RPMLeft_inst->getData()) / 2.0;   // Rotacional
            float VelTrans = (speed->RPMRight_inst->getData() + speed->RPMLeft_inst->getData()) / 2.0; // Translacional
            speed->VelTrans->setData(VelTrans);
            speed->VelRot->setData(VelRot);

            // Cálculo do erro
            float SensorArrayPosition = SensorsService::getInstance()->getArraySensors(); // posição do robô
            float erro = ARRAY_TARGET_POSITION - SensorArrayPosition;
            soma_erro += erro;
            DataPID->setpoint->setData(ARRAY_TARGET_POSITION);
            DataPID->erro->setData(erro);
            DataPID->erroquad->setData(soma_erro*soma_erro);

            // Cálculo do PID para posicionar o robô  na linha
            PID_Consts pidConsts = getTrackSegmentPID(RealTracklen, estado, DataPID);
            float pid = calculatePID(pidConsts, erro, soma_erro, SensorArrayPosition, &lastSensorArrayPosition);
            
            DataPID->output->setData(pid);

            // Calculo de velocidade do motor
            speed->right->setData(
                constrain(speed->linearSpeed->getData() + pid, MIN_SPEED, MAX_SPEED));

            speed->left->setData(
                constrain(speed->linearSpeed->getData() - pid, MIN_SPEED, MAX_SPEED));

            bool openloopControl = status->OpenLoopControl->getData();
            uint16_t OpenLoopThreshold = status->OpenLoopTreshold->getData();
            if(abs(erro) >= OpenLoopThreshold && openloopControl)
            {
                int8_t min = speed->OpenLoopMinSpeed->getData();
                int8_t max = speed->OpenLoopMaxSpeed->getData();
                OpenLoopControl(erro, max, min);         
            }
            ControlMotors(speed->left->getData(), speed->right->getData()); // Altera a velocidade dos motores

            // Define a aceleração do robô
            accel = speed->accelration->getData();
            if (estado == CAR_ENC_READING_BEFORE_FIRSTMARK)
            {
                accel = speed->initialaccelration->getData();
            }
            desaccel = speed->desaccelration->getData();

            // Altera a velocidade linear do carrinho
            TrackSegment tracksegment = (TrackSegment)status->TrackStatus->getData();
            speedTarget = getTargetSpeed(tracksegment, estado, speed);

            // Rampeia a velocidade linear do robô
            float speedValue = speed->linearSpeed->getData();
            float newSpeed = calculateSpeed(accel, speedValue);

            if (speedValue >= speedTarget)
                newSpeed = calculateSpeed(-desaccel, speedValue);

            storingSpeedValue(newSpeed);

            if (iloop > 200)
            {
                ESP_LOGD(GetName().c_str(), "dataPID: %.2f", DataPID->output->getData());
                ESP_LOGD(GetName().c_str(), "Kd: %.4f | Kp: %.4f\n", pidConsts.KD, pidConsts.KP);
                ESP_LOGD(GetName().c_str(), "speedLeft: %.2f | speedRight: %.2f", speed->left->getData(), speed->right->getData());
                ESP_LOGD(GetName().c_str(), "VelTrans: %.2f | VelRot: %.2f\n", VelTrans, VelRot);
                ESP_LOGD(GetName().c_str(), "speedMin: %d | speedMax: %d | speedBase: %d", speedMin, speedMax, speedBase);
                iloop = 0;
            }
            iloop++;
        }
    }
}

// Rotina que é executada com base no acionamento do timer
bool IRAM_ATTR PIDService::timer_group_isr_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) 
{
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(SemaphoreTimer, &high_task_awoken);
    return (high_task_awoken == pdTRUE);
}

void PIDService::TimerInit(float periodSeconds)
{
    // Configura o timer
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_FREQ, // 1MHz, 1 tick=1us
    };
    
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = (uint64_t) (periodSeconds * TIMER_FREQ), // define até quando o timer deve contar
        .reload_count = 0, // Quando a contagem alcançar o valor definido o contador será zerado
    };
    alarm_config.flags.auto_reload_on_alarm = true; // habilita auto-reload
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_group_isr_callback, // registra user callback
    };

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer, 0));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}
void PIDService::resetGlobalVariables()
{
    soma_erro = 0;
    speedTarget = 0;
    speed->linearSpeed->setData(0);
}

void PIDService::OpenLoopControl(float erro, int max, int min)
{
    
    speed->right->setData(constrain(max, MIN_SPEED, MAX_SPEED));
    speed->left->setData(constrain(min, MIN_SPEED, MAX_SPEED));
    if(erro < 0)
    {
        speed->right->setData(constrain(min, MIN_SPEED, MAX_SPEED));
        speed->left->setData(constrain(max, MIN_SPEED, MAX_SPEED));
    }
}

float PIDService::calculatePID(PID_Consts pidConsts, float erro, float somaErro, float input, float *lastInput)
{
    float P = pidConsts.KP * erro;
    float I = pidConsts.KI * somaErro * TaskDelaySeconds;
    I = constrain(I, MIN_SPEED, MAX_SPEED);
    float D = pidConsts.KD * (*lastInput - input) / TaskDelaySeconds;

    *lastInput = input;
    
    return P + I + D;
}

void PIDService::ControlMotors(float left, float right)
{
    motors.motorSpeed(0, left);
    motors.motorSpeed(1, right); 
}

float PIDService::calculateSpeed(float acceleration, float speedValue)
{
    float newSpeed = speedValue + (acceleration * TaskDelaySeconds);
    return constrain(newSpeed, speedValue, speedTarget);
}

void PIDService::storingSpeedValue(float newSpeed)
{
    speed->linearSpeed->setData(newSpeed);
}
