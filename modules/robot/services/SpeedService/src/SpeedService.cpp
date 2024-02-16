#include "SpeedService.hpp"

SpeedService::SpeedService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();
    this->speed = robot->getSpeed();

    this->diameterWheel = speed->WheelDiameter->getData(); 
    this->diameterRobot = speed->RobotDiameter->getData();

    // GPIOs dos encoders dos encoders dos motores
    //enc_motEsq.attachFullQuad(ENC_MOT_ESQ_B, ENC_MOT_ESQ_A);
    //enc_motDir.attachFullQuad(ENC_MOT_DIR_B, ENC_MOT_DIR_A);

    enc_motEsq.attachFullQuad(ENC_MOT_ESQ_A, ENC_MOT_ESQ_B);
    enc_motDir.attachFullQuad(ENC_MOT_DIR_A, ENC_MOT_DIR_B);

    MPR_Mot = speed->MPR->getData();
};

void SpeedService::Run()
{
    // Variavel necerraria para funcionaliade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Variavel contendo quantidade de pulsos inicial do carro
    initialTicksCar = xTaskGetTickCount();

    // Quando for começar a utilizar, necessario limpeza da contagem.
    resetEncondersValue();

    // Loop
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);

        estado = (CarState)robot->getStatus()->robotState->getData();

        if (estado == CAR_STOPPED && robot->getMappingData()->rightMarks->getData() == 0)
        {
            resetEncondersValue();
            lastPulseLeft = 0;
            lastPulseRight = 0;
            lastPulseLeftToPositionCalculus = 0;
            lastPulseRightToPositionCalculus = 0;
        }

        EncodersMutex.lock();
        float deltaEncDir = (enc_motDir.getCount() - lastPulseRightToPositionCalculus);
        float deltaEncEsq = (enc_motEsq.getCount() - lastPulseLeftToPositionCalculus);
        lastPulseRightToPositionCalculus = enc_motDir.getCount();
        lastPulseLeftToPositionCalculus = enc_motEsq.getCount();
        EncodersMutex.unlock();
        
        deltaS = ((deltaEncDir+deltaEncEsq) * M_PI * diameterWheel )/((float)2.0*MPR_Mot);
        deltaA = ((deltaEncEsq-deltaEncDir) * M_PI * diameterWheel )/((float)MPR_Mot*diameterRobot); 

        if (estado != CAR_STOPPED)
        {
            Ang += deltaA;
            DeltaPositionX = abs(deltaS) * cos(Ang);
            DeltaPositionY = abs(deltaS) * sin(Ang);
            positionX += DeltaPositionX;
            positionY += DeltaPositionY; 
            speed->positionX->setData(positionX / 10.0); // cm
            speed->positionY->setData(positionY / 10.0); // cm
        }

        if (iloop >= 100)
        {
            ESP_LOGD(GetName().c_str(), "encDir: %ld | encEsq: %ld | encmedia: %ld", enc_motDir.getCount(), enc_motEsq.getCount(), speed->EncMedia->getData());
            ESP_LOGD(GetName().c_str(), "deltaX: %.4f | deltaY: %.4f |  PositionX: %.4f | PositionY: %.4f", DeltaPositionX, DeltaPositionY, positionX,positionY);
            ESP_LOGD(GetName().c_str(), "Soma: %d - VelEncDir: %d | VelEncEsq: %d", (speed->RPMRight_inst->getData() + speed->RPMLeft_inst->getData()), speed->RPMRight_inst->getData(), speed->RPMLeft_inst->getData());
            ESP_LOGD(GetName().c_str(), "MPR: %d",speed->MPR->getData());
            iloop = 0;
        }
        iloop++;
    }
}

void SpeedService::resetEncondersValue() {
    std::lock_guard<std::mutex> myLock(EncodersMutex);
    enc_motEsq.clearCount();
    enc_motDir.clearCount();
    lastPulseLeft = 0;
    lastPulseRight = 0;
    lastPulseLeftToPositionCalculus = 0;
    lastPulseRightToPositionCalculus = 0;
    storeEncCount(0,0);
}

int16_t SpeedService::CalculateWheelSpeed(int32_t ActualPulsesCount, int32_t lastPulsesCount, int64_t dt_MicroSeconds)
{
    float dt_Minutes = dt_MicroSeconds / MICROSECONDS_TO_MINUTES_RATIO;
    int16_t WheelSpeed = (ActualPulsesCount - lastPulsesCount) / (MPR_Mot * dt_Minutes);
    return WheelSpeed;

}
void SpeedService::storeWheelsSpeed(int16_t LeftWheelSpeed,int16_t RightWheelSpeed)
{
    speed->RPMLeft_inst->setData(LeftWheelSpeed);
    speed->RPMRight_inst->setData(RightWheelSpeed);
}
void SpeedService::storeEncCount(int16_t LeftWheelCount,int16_t RightWheelCount)
{
    speed->EncLeft->setData(LeftWheelCount);
    speed->EncRight->setData(RightWheelCount);
    speed->EncMedia->setData((LeftWheelCount + RightWheelCount)/2);
}
void SpeedService::MeasureWheelsSpeed()
{
    std::lock_guard<std::mutex> myLock(EncodersMutex);
    int64_t deltaTime = (esp_timer_get_time() - lastTimeWheelsSpeedMeasured);
    lastTimeWheelsSpeedMeasured = esp_timer_get_time();

    int16_t LeftWheelSpeed = CalculateWheelSpeed(enc_motEsq.getCount(), lastPulseLeft, deltaTime);
    int16_t RightWheelSpeed = CalculateWheelSpeed(enc_motDir.getCount(), lastPulseRight, deltaTime);
    lastPulseLeft = enc_motEsq.getCount();
    lastPulseRight = enc_motDir.getCount();

    storeWheelsSpeed(LeftWheelSpeed, RightWheelSpeed);
    storeEncCount(lastPulseLeft, lastPulseRight);
}

int16_t SpeedService::CalculateRobotLinearSpeed()
{
    float MaxMotorSpeed = speed->MotorMaxSpeed->getData();
    int16_t RightWheelSpeed =  speed->RPMRight_inst->getData();
    int16_t LeftWheelSpeed = speed->RPMLeft_inst->getData();
    return  100.0 * ((RightWheelSpeed + LeftWheelSpeed) / (2.0 * MaxMotorSpeed));
}

int16_t SpeedService::CalculateOffsetToDecelerate(int16_t FinalSpeed, float DecelerationAdjustableGain)
{
    int16_t CurrentSpeed = CalculateRobotLinearSpeed();
    int16_t offset = (pow(FinalSpeed,2) - pow(CurrentSpeed,2)) * DecelerationAdjustableGain;
    return offset;
}