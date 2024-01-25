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
    // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Loop
    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);

        getLatSensors(); // leitura dos sensores laterais
        processsideSensors();
    }
}

void SensorsService::calibrateAllSensors()
{
    calibrateSensors(frontSensors, LED_COLOR_BLUE);
    calibrateSensors(sideSensors, LED_COLOR_RED);
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
}

void calibrateSensors(QTRSensors sensorIR, LedColor color) {
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, color, 1);
    for (uint16_t i = 0; i < 40; i++)
    {   
        ESP_LOGD(GetName().c_str(), "(%p) | sensor: (%p)", this, &sensorIR);
        sensorIR.calibrate();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void SensorsService::getLatSensors() // função leitura dos sensores
{
    // Arrays para armazenar leitura bruta dos sensores laterais
    uint16_t sideSensorschannels[sideSensors.getSensorCount()];

    sideSensors.readCalibrated(sideSensorschannels);                                                                 // leitura dos sensores laterais
    std::vector<uint16_t> sideSensorschannelsVec(sideSensorschannels, sideSensorschannels + sideSensors.getSensorCount());         // vector(array) com os valores dos sensores laterais

    // armazenando da leitura bruta do sensor lateral no objeto Braia
    robot->getsideSensors()->setChannels(sideSensorschannelsVec);

    if (latloop >= 100)
    {
        ESP_LOGD(GetName().c_str(), "Laterais -  Esquerdo: %d | Direito : %d ", robot->getsideSensors()->getChannel(0), robot->getsideSensors()->getChannel(1));
        latloop = 0;
    }
    latloop++;
}

uint16_t SensorsService::getArraySensors() // função leitura dos sensores frontais
{
    // Arrays para armazenar leitura bruta dos sensores array
    uint16_t frontSensorschannels[frontSensors.getSensorCount()];

    if(status->LineColorBlack->getData()) robot->getfrontSensors()->setLine(frontSensors.readLineBlack(frontSensorschannels));
    else robot->getfrontSensors()->setLine(frontSensors.readLineWhite(frontSensorschannels));
    // cálculo dos valores do sensor array
    std::vector<uint16_t> frontSensorsValues(frontSensorschannels, frontSensorschannels + frontSensors.getSensorCount()); // vector(array) com os valores do sensor array       // vector(array) com os valores dos sensores laterais

    // armazenando da leitura bruta do sensor array no objeto Braia
    robot->getfrontSensors()->setChannels(frontSensorsValues);

    if (sloop >= 100)
    {
        ESP_LOGD(GetName().c_str(), "Array: %d | %d | %d | %d | %d | %d | %d | %d ", frontSensorschannels[0], frontSensorschannels[1], frontSensorschannels[2], frontSensorschannels[3], frontSensorschannels[4], frontSensorschannels[5], frontSensorschannels[6], frontSensorschannels[7]);
        ESP_LOGD(GetName().c_str(), "Linha: %d", robot->getfrontSensors()->getLine());
        sloop = 0;
    }
    sloop++;

    return robot->getfrontSensors()->getLine();
}

void SensorsService::processsideSensors()
{
    uint16_t leftSideSensoRead = sideSensorsData->getChannel(0);
    uint16_t rightSideSensoRead = sideSensorsData->getChannel(1);
    
    sensorsReadNumber++; 
    sumReadLeftSensor += leftSideSensoRead;
    sumReadRightSensor += rightSideSensoRead;


    targetNumberSensorReadsToMean = 1;
    if(status->robotState->getData() == CAR_MAPPING)
        targetNumberSensorReadsToMean = MappingData->targetNumberSensorReadsToMean->getData();
    
   

    if (sensorsReadNumber >= targetNumberSensorReadsToMean)  //valor definido na dashboard
    {
        int meanSensEsq = (sumReadLeftSensor/sensorsReadNumber);
        int meanSensDir = (sumReadRightSensor/sensorsReadNumber);
        
        processSensorData(meanSensEsq, meanSensDir);
       
    }
}



void SensorsService::processSensorData(int meanSensEsq, int meanSensDir) {
    if (isWhite(meanSensEsq) || isWhite(meanSensDir)) {
        processWhiteSensors(meanSensEsq, meanSensDir);
    } else {
        processNoWhiteSensors();
    }
    resetSensorData();
}

void SensorsService::processWhiteSensors(int meanSensEsq, int meanSensDir) {
    if (isWhite(meanSensEsq) && isBlack(meanSensDir)) {
        processLeftWhiteRightBlack();
    } else if (isWhite(meanSensDir) && isBlack(meanSensEsq)) {
        processRightWhiteLeftBlack();
    } else if (isWhite(meanSensEsq) && isWhite(meanSensDir)) {
        processBothWhiteSensors();
    }
}

bool SensorsService::isWhite(int sensorValue) {
    return sensorValue < 300;
}

bool SensorsService::isBlack(int sensorValue) {
    return sensorValue > 600;
}

void SensorsService::processNoWhiteSensors() {
    if (MappingData->latDirPass->getData() || MappingData->latEsqPass->getData()) {
        LEDsService::getInstance()->LedComandSend(LED_POSITION_LEFT, LED_COLOR_BLACK, 1);
        LEDsService::getInstance()->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_BLACK, 1);
    }

    MappingData->latDirPass->setData(false);
    MappingData->latEsqPass->setData(false);
}

void SensorsService::processLeftWhiteRightBlack() {
    if (!MappingData->latEsqPass->getData() && status->robotState->getData() != CAR_STOPPED) {
        MappingData->leftPassedInc();
    }

    MappingData->latEsqPass->setData(true);
    MappingData->latDirPass->setData(false);

    LEDsService::getInstance()->LedComandSend(LED_POSITION_LEFT, LED_COLOR_RED, 1);
    LEDsService::getInstance()->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_BLACK, 1);
}

void SensorsService::processRightWhiteLeftBlack() {
    if (!MappingData->latDirPass->getData() && status->robotState->getData() != CAR_STOPPED) {
        MappingData->rightPassedInc();
    }

    MappingData->latDirPass->setData(true);
    MappingData->latEsqPass->setData(false);

    LEDsService::getInstance()->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_RED, 1);
    LEDsService::getInstance()->LedComandSend(LED_POSITION_LEFT, LED_COLOR_BLACK, 1);
}

void SensorsService::processBothWhiteSensors() {
    if ((MappingData->latDirPass->getData() && !MappingData->latEsqPass->getData()) ||
        (MappingData->latEsqPass->getData() && !MappingData->latDirPass->getData())) {
        LEDsService::getInstance()->LedComandSend(LED_POSITION_LEFT, LED_COLOR_BLACK, 1);
        LEDsService::getInstance()->LedComandSend(LED_POSITION_RIGHT, LED_COLOR_BLACK, 1);
    }

    MappingData->latDirPass->setData(true);
    MappingData->latEsqPass->setData(true);
}

void SensorsService::resetSensorData() {
    sensorsReadNumber = 0;
    sumReadRightSensor = 0;
    sumReadLeftSensor = 0;
}
