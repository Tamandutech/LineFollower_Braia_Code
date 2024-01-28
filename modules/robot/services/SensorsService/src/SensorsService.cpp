#include "SensorsService.hpp"

SensorsService::SensorsService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();

    MappingData = robot->getMappingData();
    sideSensorsData = robot->getsideSensors();
    status = robot->getStatus();

    // Definindo GPIOs e configs para sensor Array
    frontSensors.setTypeMCP3008();
    frontSensors.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, 8, (gpio_num_t)ADC_DOUT, (gpio_num_t)ADC_DIN, (gpio_num_t)ADC_CLK, (gpio_num_t)ADC_CS, 1350000, VSPI_HOST);
    frontSensors.setSamplesPerSensor(5);

    // Definindo GPIOs e configs para sensor Lateral
    sideSensors.setTypeAnalogESP(robot->getADC_handle());
    sideSensors.setSensorPins((const adc_channel_t[]){(adc_channel_t)SL1, (adc_channel_t)SL2}, 2);
    sideSensors.setSamplesPerSensor(5);
    calibrateAllSensors();
}

void SensorsService::Run()
{
    // Variavel necessária para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);

        UpdateSideSensors();
        processSideSensors();
    }
}

void SensorsService::calibrateAllSensors()
{
    calibrateSensors(&frontSensors, LED_COLOR_BLUE);
    calibrateSensors(&sideSensors, LED_COLOR_RED);
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
}

void SensorsService::calibrateSensors(QTRSensors *sensorIR, LedColor color)
{
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, color, 1);
    for (uint16_t i = 0; i < 40; i++)
    {
        ESP_LOGD(GetName().c_str(), "(%p) | sensor: (%p)", this, sensorIR);
        sensorIR->calibrate();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void SensorsService::UpdateSideSensors()
{
    // Arrays para armazenar leitura bruta dos sensores laterais
    uint16_t sideSensorschannels[sideSensors.getSensorCount()];

    sideSensors.readCalibrated(sideSensorschannels);                                                                       // leitura dos sensores laterais
    std::vector<uint16_t> sideSensorschannelsVec(sideSensorschannels, sideSensorschannels + sideSensors.getSensorCount()); // vector(array) com os valores dos sensores laterais

    // armazenando da leitura bruta do sensor lateral
    robot->getsideSensors()->setChannels(sideSensorschannelsVec);

    if (latloop >= 100)
    {
        ESP_LOGD(GetName().c_str(), "Laterais -  Esquerdo: %d | Direito : %d ", robot->getsideSensors()->getChannel(0), robot->getsideSensors()->getChannel(1));
        latloop = 0;
    }
    latloop++;
}

uint16_t SensorsService::UpdateFrontSensors() // função leitura dos sensores frontais
{
    // Arrays para armazenar leitura bruta dos sensores array
    uint16_t frontSensorschannels[frontSensors.getSensorCount()];

    if (status->LineColorBlack->getData())
        robot->getfrontSensors()->setWeightedMean(frontSensors.readLineBlack(frontSensorschannels));
    else
        robot->getfrontSensors()->setWeightedMean(frontSensors.readLineWhite(frontSensorschannels));

    std::vector<uint16_t> frontSensorsValues(frontSensorschannels, frontSensorschannels + frontSensors.getSensorCount()); // vector(array) com os valores do sensor array       // vector(array) com os valores dos sensores laterais

    // armazenando da leitura bruta do sensor array
    robot->getfrontSensors()->setChannels(frontSensorsValues);

    if (sloop >= 100)
    {
        ESP_LOGD(GetName().c_str(), "Array: %d | %d | %d | %d | %d | %d | %d | %d ", frontSensorschannels[0], frontSensorschannels[1], frontSensorschannels[2], frontSensorschannels[3], frontSensorschannels[4], frontSensorschannels[5], frontSensorschannels[6], frontSensorschannels[7]);
        ESP_LOGD(GetName().c_str(), "Linha: %d", robot->getfrontSensors()->getWeightedMean());
        sloop = 0;
    }
    sloop++;

    return robot->getfrontSensors()->getWeightedMean();
}

void SensorsService::processSideSensors()
{
    uint16_t leftSideSensoRead = sideSensorsData->getChannel(0);
    uint16_t rightSideSensoRead = sideSensorsData->getChannel(1);

    sensorsReadNumber++;
    sumReadLeftSensor += leftSideSensoRead;
    sumReadRightSensor += rightSideSensoRead;

    targetNumberSensorReadsToMean = 1;
    if (status->robotState->getData() == CAR_MAPPING)
        targetNumberSensorReadsToMean = MappingData->targetNumberSensorReadsToMean->getData();

    if (sensorsReadNumber >= targetNumberSensorReadsToMean) // valor definido na dashboard
    {
        int meanLeftSideSensor = (sumReadLeftSensor / sensorsReadNumber);
        int meanRightSideSensor = (sumReadRightSensor / sensorsReadNumber);

        processSensorData(meanLeftSideSensor, meanRightSideSensor);
    }
}

bool SensorsService::isWhite(int sensorValue)
{
    return sensorValue < 300;
}

bool SensorsService::isBlack(int sensorValue)
{
    return sensorValue > 600;
}

void SensorsService::processSensorData(int meanLeftSideSensor, int meanRightSideSensor)
{
    if (isWhite(meanLeftSideSensor) || isWhite(meanRightSideSensor))
        processSensorsReadingMark(meanLeftSideSensor, meanRightSideSensor);
    else
        processSensorsReadingNothing();

    resetSensorData();
}

void SensorsService::processSensorsReadingMark(int meanLeftSideSensor, int meanRightSideSensor)
{
    if (isWhite(meanLeftSideSensor) && isBlack(meanRightSideSensor))
        processLefSensorReadingMark();
    else if (isWhite(meanRightSideSensor) && isBlack(meanLeftSideSensor))
        processRightSensorReadingMark();
    else if (isWhite(meanLeftSideSensor) && isWhite(meanRightSideSensor))
        processBothSensorsReadingMark();
}

// TODO: Remover
void SensorsService::processLefSensorReadingMark()
{
    if (!MappingData->leftSensorReadingMark->getData())
    {
        if (status->robotState->getData() != CAR_STOPPED)
            MappingData->addMarkOnLeftSensor();

        MappingData->leftSensorReadingMark->setData(true);
        MappingData->rigthSensorReadingMark->setData(false);

        LEDsService::getInstance()->LedComandSend(LED_POSITION_LEFT, LED_COLOR_RED, 1);
        LEDsService::getInstance()->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_BLACK, 1);
    }
}

// TODO: Remover
void SensorsService::processRightSensorReadingMark()
{
    if (!MappingData->rigthSensorReadingMark->getData())
    {
        if (status->robotState->getData() != CAR_STOPPED)
            MappingData->addMarkOnRightSensor();

        MappingData->rigthSensorReadingMark->setData(true);
        MappingData->leftSensorReadingMark->setData(false);

        LEDsService::getInstance()->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_RED, 1);
        LEDsService::getInstance()->LedComandSend(LED_POSITION_LEFT, LED_COLOR_BLACK, 1);
    }
}

void SensorsService::processSensorreadingMark(DataAbstract<bool> *sensorReading, Sensors sensorSide)
{
    if (sensorWasNotReadingMark(sensorReading->getData()))
    {
        addMarkOnSensor(sensorSide);
        setSideSensorReading(sensorSide);
        setColorSideLED(sensorSide);
    }
}

bool SensorsService::sensorWasNotReadingMark(DataAbstract<bool> *sensorReading)
{
    return !sensorReading->getData();
}

void SensorsService::addMarkOnSensor(Sensors sensorSide)
{
    if (status->robotState->getData() != CAR_STOPPED)
    {
        if (sensorSide == LEFT)
            MappingData->addMarkOnLeftSensor();
        else
            MappingData->addMarkOnRightSensor();
    }
}

void SensorsService::setSideSensorReading(Sensors sensorSide)
{
    MappingData->leftSensorReadingMark->setData(sensorSide == LEFT || sensorSide == BOTH);
    MappingData->rigthSensorReadingMark->setData(sensorSide == RIGHT || sensorSide == BOTH);
}

void SensorsService::setColorSideLED(Sensors sensorSide)
{
    LEDsService *ledService = LEDsService::getInstance();

    ledService->LedComandSend(LED_POSITION_LEFT, LED_COLOR_BLACK, 1);
    ledService->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_BLACK, 1);

    if (sensorSide == LEFT)
        ledService->LedComandSend(LED_POSITION_LEFT, LED_COLOR_RED, 1);
    else if ((sensorSide == RIGHT))
        ledService->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_RED, 1);
}

// TODO: Remover
void SensorsService::processBothSensorsReadingMark()
{
    if ((MappingData->rigthSensorReadingMark->getData() && !MappingData->leftSensorReadingMark->getData()) ||
        (MappingData->leftSensorReadingMark->getData() && !MappingData->rigthSensorReadingMark->getData()))
    {
        LEDsService::getInstance()->LedComandSend(LED_POSITION_LEFT, LED_COLOR_BLACK, 1);
        LEDsService::getInstance()->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_BLACK, 1);
    }

    MappingData->rigthSensorReadingMark->setData(true);
    MappingData->leftSensorReadingMark->setData(true);
}


// TODO: Remover
void SensorsService::processSensorsReadingNothing()
{
    if (MappingData->rigthSensorReadingMark->getData() || MappingData->leftSensorReadingMark->getData())
    {
        LEDsService::getInstance()->LedComandSend(LED_POSITION_LEFT, LED_COLOR_BLACK, 1);
        LEDsService::getInstance()->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_BLACK, 1);
    }

    MappingData->rigthSensorReadingMark->setData(false);
    MappingData->leftSensorReadingMark->setData(false);
}

void SensorsService::resetSensorData()
{
    sensorsReadNumber = 0;
    sumReadRightSensor = 0;
    sumReadLeftSensor = 0;
}
