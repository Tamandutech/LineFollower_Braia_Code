#include "MappingService.hpp"

MappingService::MappingService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();

    speedMapping = robot->getSpeed();
    sLat = robot->getsLat();
    MappingData = robot->getMappingData();
    status = robot->getStatus();

    esp_log_level_set(GetName().c_str(), ESP_LOG_ERROR);
};

esp_err_t MappingService::startNewMapping()
{
    ESP_LOGD(GetName().c_str(), "Iniciando novo mapeamento.");

    this->rightMarksToStop = MappingData->MarkstoStop->getData();

    EncLeft = 0;
    EncRight = 0;
    lastEncLeft = 0;
    lastEncRight = 0;
    lastmarkPosition = 0;
    currentMark.markPosition = 0;
    currentMark.trackSegmentBeforeMark = LONG_LINE;
    currentMark.timeUntilMarkReading = 0;
    currentMark.offsetMarkPosition = 0;


    MappingData->rightMarks->setData(0);
    MappingData->leftMarks->setData(0);

    MappingData->TrackSideMarks->clearAllData();

    this->Start();

    return ESP_OK;
}

esp_err_t MappingService::stopNewMapping()
{
    ESP_LOGD(GetName().c_str(), "Parando novo mapeamento.");

    status->robotState->setData(CAR_STOPPED);
    DataManager::getInstance()->saveAllParamDataChanged();
    this->Cleanup();
    this->saveMapping();
    ESP_LOGD(GetName().c_str(), "Parada do novo mapeamento finalizada");
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
    return ESP_OK;
}

esp_err_t MappingService::loadMapping()
{
    ESP_LOGD(GetName().c_str(), "Carregando mapeamento da memória.");

    MappingData->TrackSideMarks->loadData();

    return ESP_OK;
}

esp_err_t MappingService::saveMapping()
{
    ESP_LOGD(GetName().c_str(), "Salvando mapeamento na memória.");

    MappingData->TrackSideMarks->saveData();

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

    SpeedService::getInstance()->resetEncondersValue();

    MappingData->TrackSideMarks->newData(currentMark);

    for (;;)
    {
        lastEncLeft = EncLeft;
        lastEncRight = EncRight;
        lastmarkPosition = currentMark.markPosition;

        vTaskDelay(0);
        this->Suspend();
        
        currentMark.offsetMarkPosition = 0;
        EncLeft = speedMapping->EncLeft->getData();
        EncRight = speedMapping->EncRight->getData();
        currentMark.markPosition = ((EncLeft + EncRight) / 2);
        currentMark.timeUntilMarkReading = xTaskGetTickCount()*portTICK_PERIOD_MS;

        // variação de encoder em pulsos
        uint32_t DeltaPulses = std::abs((EncRight - lastEncRight) - (EncLeft - lastEncLeft));
        // Quantidade de pulsos que o encoder precisa dar para avançar "x" milimetros
        uint32_t MilimiterInPulses = (speedMapping->MPR->getData() * MappingData->thresholdToCurve->getData()) / (M_PI * speedMapping->WheelDiameter->getData());
        uint32_t DeltaDist = ((currentMark.markPosition - lastmarkPosition) * (M_PI * speedMapping->WheelDiameter->getData())) / (speedMapping->MPR->getData()); // distância entre marcacões em mm
        if(DeltaPulses <= MilimiterInPulses)
        {
            if(DeltaDist < MappingData->MediumLineLength->getData()) currentMark.trackSegmentBeforeMark = SHORT_LINE;
            else if(DeltaDist < MappingData->LongLineLength->getData()) currentMark.trackSegmentBeforeMark = MEDIUM_LINE;
            else currentMark.trackSegmentBeforeMark = LONG_LINE;

            if(MappingData->latEsqPass->getData()) led = LED_POSITION_LEFT;
            else if(MappingData->latDirPass->getData()) led = LED_POSITION_RIGHT;
            color = LED_COLOR_GREEN;
        }
        else
        {
            if(DeltaDist < MappingData->MediumCurveLength->getData()) currentMark.trackSegmentBeforeMark = SHORT_CURVE;
            else if(DeltaDist < MappingData->LongCurveLength->getData()) currentMark.trackSegmentBeforeMark = MEDIUM_CURVE;
            else currentMark.trackSegmentBeforeMark = LONG_CURVE;

            if(MappingData->latEsqPass->getData()) led = LED_POSITION_LEFT;
            else if(MappingData->latDirPass->getData()) led = LED_POSITION_RIGHT;
            color = LED_COLOR_RED;
        }
        MappingData->TrackSideMarks->newData(currentMark);
        
        LEDsService::getInstance()->LedComandSend(led, color, 1);
        

        ESP_LOGD(GetName().c_str(), "Marcação: MapEncLeft: %ld, MapEncRight: %ld, markPosition: %ld, timeUntilMarkReading: %lu", EncLeft, EncRight, currentMark.markPosition, currentMark.timeUntilMarkReading);

        if (rightMarksToStop <= MappingData->rightMarks->getData())
        {
            ESP_LOGD(GetName().c_str(), "Mapeamento finalizado.");
            this->stopNewMapping();
            break;
        }
    }
}