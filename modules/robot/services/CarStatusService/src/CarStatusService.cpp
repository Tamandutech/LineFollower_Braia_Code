#include "CarStatusService.hpp"

SemaphoreHandle_t CarStatusService::SemaphoreButton;

void IRAM_ATTR CarStatusService::startRobotWithBootButton(void *arg)
{
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(SemaphoreButton, &high_task_awoken);
}

CarStatusService::CarStatusService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();
    this->status = robot->getStatus();
    this->speed = robot->getSpeed();
    this->MappingData = robot->getMappingData();

    mappingService = MappingService::getInstance();

    status->robotState->setData(CAR_STOPPED);
    status->currentTrackSegment->setData(DEFAULT_TRACK);
    status->transitionTrackSegment->setData(DEFAULT_TRACK);

    lastPaused = status->robotPaused->getData();
    lastRobotState = (CarState)status->robotState->getData();
    lastTrack = (TrackSegment)status->currentTrackSegment->getData();

    SemaphoreButton = xSemaphoreCreateBinary();
    configExternInterruptToReadButton(GPIO_NUM_0);

}

void CarStatusService::defineIfRobotWillStartMappingMode()
{
    MappingData->TrackSideMarks->loadData();
    initialRobotState = CAR_MAPPING;
    if (MappingData->TrackSideMarks->getSize() > 0)
        startFollowingDefinedMapping();
}

void CarStatusService::startFollowingDefinedMapping()
{
    status->transitionTrackSegment->setData(SHORT_LINE);
    status->currentTrackSegment->setData(SHORT_LINE);

    initialRobotState = CAR_ENC_READING_BEFORE_FIRSTMARK;
    numMarks = MappingData->TrackSideMarks->getSize();
    finalMark = MappingData->TrackSideMarks->getData(numMarks - 1);
}

void CarStatusService::configExternInterruptToReadButton(gpio_num_t interruptPort)
{
    gpio_config_t interruptConfig = {};
    interruptConfig.intr_type = GPIO_INTR_NEGEDGE;
    interruptConfig.pin_bit_mask = (1ULL << interruptPort);
    interruptConfig.mode = GPIO_MODE_INPUT;
    interruptConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    interruptConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;

    gpio_config(&interruptConfig);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(interruptPort, startRobotWithBootButton, NULL);
}

void CarStatusService::Run()
{
    // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a conGetName().c_str()em de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();

    waitPressBootButtonToStart();

    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_RED, 1);

    vTaskDelay(1500 / portTICK_PERIOD_MS);

    if (!gpio_get_level(GPIO_NUM_0) && MappingData->TrackSideMarks->getSize() > 0 && !status->TunningMode->getData() && status->HardDeleteMap->getData())
        deleteMappingIfBootButtonIsPressed();
    
    ESP_LOGD(GetName().c_str(), "Iniciando delay de 1500ms");
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    if (status->TunningMode->getData())
        setTuningMode();
    else
    {
        defineIfRobotWillStartMappingMode();
        if (initialRobotState == CAR_MAPPING)
            startMappingTheTrack();
    }

    status->robotState->setData(initialRobotState);

    // Loop
    for (;;)
    {

        vTaskDelayUntil(&xLastWakeTime, 30 / portTICK_PERIOD_MS);
        pulsesBeforeCurve = MappingData->PulsesBeforeCurve->getData();
        currentRobotState = (CarState)status->robotState->getData();

        if (status->robotPaused->getData())
            lastPaused = true;

        if (passedFirstMark())
            resetEnconderInFirstMark();

        if (trackSegmentChanged() || RobotStateChanged())
        {
            LedColor color = defineLedColor();
            setColorBrightness(color);
        }

        if (currentRobotState == CAR_TUNING && !status->TunningMode->getData())
            stopTunningMode();

        if (currentRobotState == CAR_ENC_READING)
        {
            robotPosition = (speed->EncRight->getData() + speed->EncLeft->getData()) / 2;
            if (robotPosition >= finalMark.markPosition)
            {
                ESP_LOGD(GetName().c_str(), "Parando o robô");

                robot->getStatus()->robotState->setData(CAR_STOPPED);
                DataManager::getInstance()->saveAllParamDataChanged();
                LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
            }
            if (robotPosition < finalMark.markPosition)
            {
                // define o status do carrinho se o mapeamento não estiver ocorrendo
                for (int mark = 0; mark < numMarks - 1; mark++)
                {
                    MapData previousMark = MappingData->TrackSideMarks->getData(mark);
                    MapData currentMark = MappingData->TrackSideMarks->getData(mark + 1);

                    if (robotPosition >= previousMark.markPosition && robotPosition <= currentMark.markPosition)
                    {
                        defineTrackSegment(currentMark);

                        transition = false;

                        int16_t previousMarkoffset = previousMark.offsetMarkPosition;
                        int16_t currentMarkOffset = currentMark.offsetMarkPosition;
                        
                        TrackSegment previousTrack = getTrackSegment(previousMark);
                        if (robotPosition < (previousMark.markPosition + previousMarkoffset))
                        {
                            transition = true;
                            transitionTrackSegment = previousTrack;
                        }
                        if (mark + 2 < numMarks)
                        {
                            MapData nextMark = MappingData->TrackSideMarks->getData(mark + 2);
                            TrackSegment currentTrack = getTrackSegment(currentMark);
                            TrackSegment nextTrack = getTrackSegment(nextMark);
                            if (isLineSegment(currentTrack) && isCurveSegment(nextTrack) && currentMarkOffset == 0)
                            {
                               currentMarkOffset = -pulsesBeforeCurve;
                            }
                            if (robotPosition > (currentMark.markPosition + currentMarkOffset))
                            {
                                transition = true;
                                transitionTrackSegment = nextTrack;
                            }
                        }
                        // Atualiza estado do robô
                        status->transitionTrackSegment->setData(transitionTrackSegment);
                        break;
                    }
                }
            }
        }
        if (iloop >= 30)
        {
            iloop = 0;
            logCarStatus();
        }
        iloop++;
    }
}

void CarStatusService::waitPressBootButtonToStart()
{
    ESP_LOGD(GetName().c_str(), "Aguardando pressionamento do botão.");
    xSemaphoreTake(SemaphoreButton, portMAX_DELAY);
}

void CarStatusService::deleteMappingIfBootButtonIsPressed()
{
    DataStorage::getInstance()->delete_data("Mapping.TrackSideMarks");
    ESP_LOGD(GetName().c_str(), "Mapeamento Deletado");
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_YELLOW, 1);
}

void CarStatusService::startMappingTheTrack()
{
    ESP_LOGD(GetName().c_str(), "Mapeamento inexistente, iniciando robô em modo mapemaneto.");
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_YELLOW, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // Começa mapeamento
    status->currentTrackSegment->setData(DEFAULT_TRACK);
    status->transitionTrackSegment->setData(DEFAULT_TRACK);
    mappingService->startNewMapping();
}

void CarStatusService::setTuningMode()
{
    initialRobotState = CAR_TUNING;
    MappingData->TrackSideMarks->clearAllData();
    numMarks = 0;
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_WHITE, 0.5);
}

bool CarStatusService::passedFirstMark()
{
    return MappingData->rightMarks->getData() >= 1 && currentRobotState == CAR_ENC_READING_BEFORE_FIRSTMARK;
}

void CarStatusService::resetEnconderInFirstMark()
{
    SpeedService::getInstance()->resetEncondersValue();
    currentRobotState = CAR_ENC_READING;
    status->robotState->setData(currentRobotState);
}

bool CarStatusService::trackSegmentChanged()
{
    return lastTrack != (TrackSegment)status->currentTrackSegment->getData() || lastTransition != transition;
}

bool CarStatusService::RobotStateChanged()
{
    return lastRobotState != currentRobotState;
}

LedColor CarStatusService::defineLedColor()
{
    lastTrack = (TrackSegment)status->currentTrackSegment->getData();
    lastTransition = transition;
    lastRobotState = currentRobotState;
    LedColor color = getStatusColor(lastRobotState, lastTrack);
    if (lastTransition)
        color = LED_COLOR_BLUE;
    return color;
}

void CarStatusService::setColorBrightness(LedColor color)
{
    float brightness = getSegmentBrightness(lastRobotState, lastTrack);
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, color, brightness);
}

void CarStatusService::logCarStatus()
{
    ESP_LOGD(GetName().c_str(), "CarStatus: %d", status->robotState->getData());
    ESP_LOGD(GetName().c_str(), "EncMedia: %ld", robotPosition);
    ESP_LOGD(GetName().c_str(), "finalMark: %ld", finalMark.markPosition);
    ESP_LOGD(GetName().c_str(), "Speed: %.2f", speed->linearSpeed->getData());
}

void CarStatusService::stopTunningMode()
{
    robot->getStatus()->robotState->setData(CAR_STOPPED);
    vTaskDelay(0);
    DataManager::getInstance()->saveAllParamDataChanged();
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
}

void CarStatusService::defineTrackSegment(MapData Mark)
{
    transitionTrackSegment = (TrackSegment) Mark.trackSegmentBeforeMark;
    status->currentTrackSegment->setData(transitionTrackSegment);
}

TrackSegment CarStatusService::getTrackSegment(MapData Mark)
{
    return (TrackSegment) Mark.trackSegmentBeforeMark;
}