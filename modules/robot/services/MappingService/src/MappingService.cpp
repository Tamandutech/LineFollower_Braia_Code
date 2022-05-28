#include "MappingService.hpp"

MappingService::MappingService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();

#ifndef ESP32_QEMU
    gpio_pad_select_gpio(0);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
#endif

    speedMapping = robot->getSpeed();
    sLat = robot->getsLat();
    latMarks = robot->getSLatMarks();
    status = robot->getStatus();
};

esp_err_t MappingService::startNewMapping(uint8_t leftMarksToStop, int32_t mediaPulsesToStop, uint32_t timeToStop)
{
    ESP_LOGD(GetName().c_str(), "Iniciando novo mapeamento.");

    status->robotIsMapping->setData(true);

    this->leftMarksToStop = leftMarksToStop;
    this->mediaPulsesToStop = mediaPulsesToStop;
    this->ticksToStop = timeToStop / portTICK_PERIOD_MS;

    tempPreviousMark.MapEncLeft = 0;
    tempPreviousMark.MapEncRight = 0;
    tempPreviousMark.MapEncMedia = 0;
    tempPreviousMark.MapStatus = CAR_IN_LINE;
    tempPreviousMark.MapTime = 0;

    latMarks->rightMarks->setData(0);
    latMarks->leftMarks->setData(0);

    tempActualMark = tempPreviousMark;

    latMarks->marks->clearAllData();

    this->Start();

    return ESP_OK;
}

esp_err_t MappingService::stopNewMapping()
{
    ESP_LOGD(GetName().c_str(), "Parando novo mapeamento.");

    status->stateMutex.lock();
    status->robotState->setData(CAR_STOPPED);
    status->robotIsMapping->setData(false);
    status->stateMutex.unlock();
    command.led[0] = LED_POSITION_FRONT;
    command.led[1] = LED_POSITION_NONE;
    command.color = LED_COLOR_BLACK;
    command.effect = LED_EFFECT_SET;
    command.brightness = 1;
    LEDsService::getInstance()->queueCommand(command);

    this->Cleanup();

    this->saveMapping();
    ESP_LOGD(GetName().c_str(), "Parada do novo mapeamento finalizada");
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
    if (status->robotIsMapping->getData() && status->robotState->getData() != CAR_STOPPED)
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

    ESP_LOGD(GetName().c_str(), "Offset iniciais: initialLeftPulses: %d, initialRightPulses: %d, initialMediaPulses: %d, initialTicks: %d", initialLeftPulses, initialRightPulses, initialMediaPulses, initialTicks);

    for (;;)
    {
        tempPreviousMark = tempActualMark;

        vTaskDelay(0);
        this->Suspend();

        tempActualMark.MapEncLeft = speedMapping->EncLeft->getData() - initialLeftPulses;
        tempActualMark.MapEncRight = speedMapping->EncRight->getData() - initialRightPulses;
        tempActualMark.MapEncMedia = ((tempActualMark.MapEncLeft + tempActualMark.MapEncRight) / 2);
        tempActualMark.MapTime = ((xTaskGetTickCount() - initialTicks) / portTICK_PERIOD_MS);

        // variação de encoder em pulsos
        tempDeltaPulses = std::abs((tempActualMark.MapEncRight - tempPreviousMark.MapEncRight) - (tempActualMark.MapEncLeft - tempPreviousMark.MapEncLeft));
        // Quantidade de pulsos que o encoder precisa dar para avançar "x" milimetros
        tempMilimiterInPulses = (speedMapping->MPR->getData() * latMarks->thresholdToCurve->getData()) / (M_PI * speedMapping->WheelDiameter->getData());

        tempActualMark.MapStatus = (tempDeltaPulses > tempMilimiterInPulses) ? CAR_IN_CURVE : CAR_IN_LINE;

        latMarks->marks->newData(tempActualMark);

        ESP_LOGD(GetName().c_str(), "Marcação: MapEncLeft: %d, MapEncRight: %d, MapEncMedia: %d, MapTime: %d, MapStatus: %d", tempActualMark.MapEncLeft, tempActualMark.MapEncRight, tempActualMark.MapEncMedia, tempActualMark.MapTime, tempActualMark.MapStatus);

        if ((leftMarksToStop <= latMarks->leftMarks->getData()) || (2 <= latMarks->rightMarks->getData()) || (mediaPulsesToStop <= tempActualMark.MapEncMedia) || (ticksToStop <= (tempActualMark.MapTime * portTICK_PERIOD_MS)))
        {
            ESP_LOGD(GetName().c_str(), "Mapeamento finalizado.");

            this->stopNewMapping();
            break;
        }
    }
}