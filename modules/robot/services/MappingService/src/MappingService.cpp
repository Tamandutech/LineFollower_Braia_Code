#include "MappingService.hpp"

std::atomic<MappingService *> MappingService::instance;
std::mutex MappingService::instanceMutex;

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

esp_err_t MappingService::startNewMapping(uint8_t leftMarksToStop, uint8_t rightMarksToStop, int32_t mediaPulsesToStop, uint32_t timeToStop)
{
    status->robotIsMapping->setData(true);

    this->leftMarksToStop = leftMarksToStop;
    this->rightMarksToStop = rightMarksToStop;
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
    status->robotIsMapping->setData(false);

    this->Cleanup();

    return ESP_OK;
}

esp_err_t MappingService::loadMapping()
{
    latMarks->marks->loadData();

    return ESP_OK;
}

esp_err_t MappingService::saveMapping()
{
    latMarks->marks->saveData();

    return ESP_OK;
}

esp_err_t MappingService::createNewMark()
{
    if (status->robotIsMapping->getData())
    {
        this->Resume();
        return ESP_OK;
    }
}

void MappingService::Run()
{
    this->Suspend();

    initialLeftPulses = speedMapping->EncLeft->getData();
    initialRightPulses = speedMapping->EncRight->getData();
    initialMediaPulses = (initialLeftPulses + initialRightPulses) / 2;
    initialTicks = xTaskGetTickCount();

    latMarks->marks->newData(tempActualMark);

    for (;;)
    {
        tempPreviousMark = tempActualMark;

        this->Suspend();

        tempActualMark.MapEncLeft = speedMapping->EncLeft->getData() - initialLeftPulses;
        tempActualMark.MapEncRight = speedMapping->EncRight->getData() - initialRightPulses;
        tempActualMark.MapEncMedia = ((tempActualMark.MapEncLeft + tempActualMark.MapEncRight) / 2) - initialMediaPulses;
        tempActualMark.MapTime = ((xTaskGetTickCount() - initialTicks) / portTICK_PERIOD_MS);

        // variação de encoder em pulsos
        tempDeltaPulses = std::abs(tempActualMark.MapEncRight - tempPreviousMark.MapEncRight) - (tempActualMark.MapEncLeft - tempPreviousMark.MapEncLeft);
        // Quantidade de pulsos que o encoder precisa dar para avançar "x" milimetros
        tempMilimiterInPulses = (speedMapping->MPR->getData() * latMarks->thresholdToCurve->getData()) / (M_PI * speedMapping->WheelDiameter->getData());

        tempActualMark.MapStatus = (tempDeltaPulses > tempMilimiterInPulses) ? CAR_IN_CURVE : CAR_IN_LINE;

        latMarks->marks->newData(tempActualMark);

        if ((leftMarksToStop <= latMarks->leftMarks->getData()) || (rightMarksToStop <= latMarks->rightMarks->getData()) || (mediaPulsesToStop <= tempActualMark.MapEncMedia) || (ticksToStop <= (tempActualMark.MapTime * portTICK_PERIOD_MS)))
        {
            this->stopNewMapping();
            break;
        }
    }
}