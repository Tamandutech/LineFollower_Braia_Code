#include "MappingService.hpp"

MappingService::MappingService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();

    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);

    speedMapping = robot->getSpeed();
    sLat = robot->getsLat();
    latMarks = robot->getSLatMarks();
    status = robot->getStatus();
};

esp_err_t MappingService::startNewMapping(uint16_t leftMarksToStop, int32_t mediaPulsesToStop, uint32_t timeToStop)
{
    ESP_LOGD(GetName().c_str(), "Iniciando novo mapeamento.");

    this->leftMarksToStop = leftMarksToStop;
    //this->rightMarksToStop = latMarks->MarkstoStop->getData();
    this->mediaPulsesToStop = mediaPulsesToStop;
    this->ticksToStop = timeToStop / portTICK_PERIOD_MS;

    EncLeft = 0;
    EncRight = 0;
    lastEncLeft = 0;
    lastEncRight = 0;
    lastEncMedia = 0;
    tempActualMark.MapEncMedia = 0;
    tempActualMark.MapTrackStatus = LONG_LINE;
    tempActualMark.MapTime = 0;
    tempActualMark.MapOffset = 0;


    latMarks->rightMarks->setData(0);
    latMarks->leftMarks->setData(0);

    latMarks->marks->clearAllData();

    this->Start();

    return ESP_OK;
}

esp_err_t MappingService::stopNewMapping()
{
    ESP_LOGD(GetName().c_str(), "Parando novo mapeamento.");

    status->stateMutex.lock();
    status->robotState->setData(CAR_STOPPED);
    DataManager::getInstance()->saveAllParamDataChanged();
    status->stateMutex.unlock();

    this->Cleanup();

    this->saveMapping();
    ESP_LOGD(GetName().c_str(), "Parada do novo mapeamento finalizada");
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
    return ESP_OK;
}

esp_err_t MappingService::loadMapping()
{
    ESP_LOGD(GetName().c_str(), "Carregando mapeamento da memória.");

    latMarks->marks->loadData();

    return ESP_OK;
}

esp_err_t MappingService::saveMapping()
{
    ESP_LOGD(GetName().c_str(), "Salvando mapeamento na memória.");

    latMarks->marks->saveData();

    return ESP_OK;
}

esp_err_t MappingService::createNewMark()
{
    if (status->robotState->getData() == CAR_MAPPING)
    {
        ESP_LOGD(GetName().c_str(), "Criando nova marcação.");

        this->Resume();
        return ESP_OK;
    }
    return ESP_OK;
}

void MappingService::Run()
{
    
    this->Suspend();

    initialLeftPulses = speedMapping->EncLeft->getData();
    initialRightPulses = speedMapping->EncRight->getData();
    initialMediaPulses = (initialLeftPulses + initialRightPulses) / 2;
    initialTicks = xTaskGetTickCount();

    latMarks->marks->newData(tempActualMark);
    ESP_LOGD(GetName().c_str(), "Offset iniciais: initialLeftPulses: %ld, initialRightPulses: %ld, initialMediaPulses: %ld, initialTicks: %lu", initialLeftPulses, initialRightPulses, initialMediaPulses, initialTicks);

    for (;;)
    {
        lastEncLeft = EncLeft;
        lastEncRight = EncRight;
        lastEncMedia = tempActualMark.MapEncMedia;

        vTaskDelay(0);
        this->Suspend();
        
        tempActualMark.MapOffset = 0;
        EncLeft = speedMapping->EncLeft->getData() - initialLeftPulses;
        EncRight = speedMapping->EncRight->getData() - initialRightPulses;
        tempActualMark.MapEncMedia = ((EncLeft + EncRight) / 2);
        tempActualMark.MapTime = ((xTaskGetTickCount() - initialTicks) * portTICK_PERIOD_MS);

        // variação de encoder em pulsos
        tempDeltaPulses = std::abs((EncRight - lastEncRight) - (EncLeft - lastEncLeft));
        // Quantidade de pulsos que o encoder precisa dar para avançar "x" milimetros
        tempMilimiterInPulses = (speedMapping->MPR->getData() * latMarks->thresholdToCurve->getData()) / (M_PI * speedMapping->WheelDiameter->getData());

        tempDeltaDist = ((tempActualMark.MapEncMedia - lastEncMedia) * (M_PI * speedMapping->WheelDiameter->getData())) / (speedMapping->MPR->getData()); // distância entre marcacões em mm
        if(tempDeltaPulses <= tempMilimiterInPulses)
        {
            if(tempDeltaDist < latMarks->thresholdMediumLine->getData()) tempActualMark.MapTrackStatus = SHORT_LINE;
            else if(tempDeltaDist < latMarks->thresholdLongLine->getData()) tempActualMark.MapTrackStatus = MEDIUM_LINE;
            else tempActualMark.MapTrackStatus = LONG_LINE;

            if(latMarks->latEsqPass->getData()) led = LED_POSITION_LEFT;
            else if(latMarks->latDirPass->getData()) led = LED_POSITION_RIGHT;
            color = LED_COLOR_GREEN;
        }
        else
        {
            if(tempDeltaDist < latMarks->thresholdMediumCurve->getData()) tempActualMark.MapTrackStatus = SHORT_CURVE;
            else if(tempDeltaDist < latMarks->thresholdLongCurve->getData()) tempActualMark.MapTrackStatus = MEDIUM_CURVE;
            else tempActualMark.MapTrackStatus = LONG_CURVE;

            if(latMarks->latEsqPass->getData()) led = LED_POSITION_LEFT;
            else if(latMarks->latDirPass->getData()) led = LED_POSITION_RIGHT;
            color = LED_COLOR_RED;
        }
        latMarks->marks->newData(tempActualMark);
        
        LEDsService::getInstance()->LedComandSend(led, color, 1);
        

        ESP_LOGD(GetName().c_str(), "Marcação: MapEncLeft: %ld, MapEncRight: %ld, MapEncMedia: %ld, MapTime: %lu", EncLeft, EncRight, tempActualMark.MapEncMedia, tempActualMark.MapTime);

        if ((leftMarksToStop <= latMarks->leftMarks->getData()) || (latMarks->MarkstoStop->getData() <= latMarks->rightMarks->getData()) || (mediaPulsesToStop <= tempActualMark.MapEncMedia) || (ticksToStop <= (tempActualMark.MapTime * portTICK_PERIOD_MS)))
        {
            ESP_LOGD(GetName().c_str(), "Mapeamento finalizado.");

            this->stopNewMapping();
            break;
        }
    }
}