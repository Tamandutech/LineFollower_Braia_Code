#include "CarStatusService.hpp"

SemaphoreHandle_t CarStatusService::SemaphoreStartRobot;

void IRAM_ATTR CarStatusService::startRobotWithBootButton(void *arg)
{
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(SemaphoreStartRobot, &high_task_awoken);
}

CarStatusService::CarStatusService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();
    this->status = robot->getStatus();
    this->speed = robot->getSpeed();
    this->MappingData = robot->getMappingData();
    printInterval = 0;

    mappingService = MappingService::getInstance();

    status->robotState->setData(CAR_STOPPED);
    status->currentTrackSegment->setData(DEFAULT_TRACK);
    status->transitionTrackSegment->setData(DEFAULT_TRACK);

    previousRobotState = (CarState)status->robotState->getData();
    previousTrack = (TrackSegment)status->currentTrackSegment->getData();
    previouslyInTransition = false;
    inTransition = false;

    previousMarkPassedNumber = 0;
    previousMarkoffset = 0;
    currentMarkOffset = 0;

    SemaphoreStartRobot = xSemaphoreCreateBinary();
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
    TotalMarksNumber = MappingData->TrackSideMarks->getSize();
    finalMark = MappingData->TrackSideMarks->getData(TotalMarksNumber - 1);
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
    TickType_t xLastWakeTime = xTaskGetTickCount();

    waitPressBootButtonToStart();

    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_RED, 1);

    vTaskDelay(1500 / portTICK_PERIOD_MS);

    if (!gpio_get_level(GPIO_NUM_0)  && !status->TunningMode->getData() && status->HardDeleteMap->getData())
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

    for (;;)
    {

        vTaskDelayUntil(&xLastWakeTime, 30 / portTICK_PERIOD_MS);
        currentRobotState = (CarState)status->robotState->getData();

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
            else
            {
                // define o trecho em que o robô, caso o robô esteja lendo o mapeamento
                for (int markNumber = 0; markNumber < TotalMarksNumber - 1; markNumber++)
                {
                    MapData previousMark = MappingData->TrackSideMarks->getData(markNumber);
                    MapData currentMark = MappingData->TrackSideMarks->getData(markNumber + 1);

                    if (robotPosition >= previousMark.markPosition && robotPosition <= currentMark.markPosition)
                    {
                        defineTrackSegment(currentMark);
                        UpdateMarkPassedNumber(markNumber);

                        TrackSegment previousTrack = getTrackSegment(previousMark);
                        TrackSegment currentTrack = getTrackSegment(currentMark);
                        TrackSegment nextTrack = DEFAULT_TRACK;
                        if(markNumber + 2 < TotalMarksNumber)
                        {
                            MapData nextMark = MappingData->TrackSideMarks->getData(markNumber + 2);
                            nextTrack = getTrackSegment(nextMark);
                        }

                        if(robotPosition <= (currentMark.markPosition + currentMarkOffset) 
                        || robotPosition >= (previousMark.markPosition + previousMarkoffset))
                        {
                            inTransition = false;
                            currentMarkOffset = currentMark.offsetMarkPosition;
                            if (isLineSegment(currentTrack) && isCurveSegment(nextTrack))
                            {
                                int16_t FinalSpeed =  getTrackSegmentSpeed(nextTrack, speed);  
                                float DecelerationOffsetGain = speed->DecelerationOffsetGain->getData();
                                currentMarkOffset += SpeedService::getInstance()->CalculateOffsetToDecelerate(FinalSpeed, DecelerationOffsetGain);
                             }
                            previousMarkoffset = previousMark.offsetMarkPosition;
                        }
                            
                        if (robotPosition > (currentMark.markPosition + currentMarkOffset) && markNumber + 2 < TotalMarksNumber)
                        {
                            inTransition = true;
                            transitionTrackSegment = nextTrack;
                        }

                        if (robotPosition < (previousMark.markPosition + previousMarkoffset))
                        {
                            inTransition = true;
                            transitionTrackSegment = previousTrack;
                        }
                        
                        // Atualiza estado do robô
                        status->transitionTrackSegment->setData(transitionTrackSegment);
                        break;
                    }
                }
            }
        }
        if (printInterval >= 30)
        {
            printInterval = 0;
            logCarStatus();
        }
        printInterval++;
    }
}

void CarStatusService::waitPressBootButtonToStart()
{
    ESP_LOGD(GetName().c_str(), "Aguardando pressionamento do botão.");
    xSemaphoreTake(SemaphoreStartRobot, portMAX_DELAY);
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
    TotalMarksNumber = 0;
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
    return previousTrack != (TrackSegment)status->currentTrackSegment->getData() || previouslyInTransition != inTransition;
}

bool CarStatusService::RobotStateChanged()
{
    return previousRobotState != currentRobotState;
}

LedColor CarStatusService::defineLedColor()
{
    TrackSegment currentTrack = (TrackSegment)status->currentTrackSegment->getData();
    previousTrack = currentTrack;
    previouslyInTransition = inTransition;
    previousRobotState = currentRobotState;
    LedColor color = getStatusColor(currentRobotState, currentTrack);
    if (inTransition)
        color = LED_COLOR_BLUE;
    return color;
}

void CarStatusService::setColorBrightness(LedColor color)
{
    TrackSegment currentTrack = (TrackSegment)status->currentTrackSegment->getData();
    float brightness = getSegmentBrightness(currentRobotState, currentTrack);
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

void CarStatusService::UpdateMarkPassedNumber(int markNumber)
{
    if(previousMarkPassedNumber != markNumber)
    {
        inTransition = false;
        previousMarkPassedNumber = markNumber;
        currentMarkOffset = 0;
        previousMarkoffset = 0;
    }
}

TrackSegment CarStatusService::getTrackSegment(MapData Mark)
{
    return (TrackSegment) Mark.trackSegmentBeforeMark;
}
