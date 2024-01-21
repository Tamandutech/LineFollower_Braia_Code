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
    this->latMarks = robot->getSLatMarks();

    mappingService = MappingService::getInstance();

    status->robotState->setData(CAR_STOPPED);
    status->RealTrackStatus->setData(DEFAULT_TRACK);
    status->TrackStatus->setData(DEFAULT_TRACK);

    lastPaused = status->robotPaused->getData();
    lastState = status->robotState->getData();
    lastTrack = (TrackSegment)status->TrackStatus->getData();

    SemaphoreButton = xSemaphoreCreateBinary();
    configExternInterruptToReadButton(GPIO_NUM_0);

    if (!status->TunningMode->getData())
        defineIfRobotWillStartMappingMode();
}

void CarStatusService::defineIfRobotWillStartMappingMode()
{
    latMarks->marks->loadData();
    initialRobotState = CAR_MAPPING;
    if (latMarks->marks->getSize() > 0)
        startFollowingDefinedMapping();
}

void CarStatusService::startFollowingDefinedMapping()
{
    status->TrackStatus->setData(SHORT_LINE);
    status->RealTrackStatus->setData(SHORT_LINE);

    initialRobotState = CAR_ENC_READING_BEFORE_FIRSTMARK;
    numMarks = latMarks->marks->getSize();
    finalMark = latMarks->marks->getData(numMarks - 1);
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

    if (!gpio_get_level(GPIO_NUM_0) && latMarks->marks->getSize() > 0 && !status->TunningMode->getData() && status->HardDeleteMap->getData())
        deleteMappingIfBootButtonIsPressed();

    ESP_LOGD(GetName().c_str(), "Iniciando delay de 1500ms");
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    if (initialRobotState == CAR_MAPPING && !status->TunningMode->getData())
        startMappingTheTrack();

    started_in_Tuning = false;
    if (status->TunningMode->getData())
        setTuningMode();

    status->robotState->setData(initialRobotState);

    // Loop
    for (;;)
    {

        vTaskDelayUntil(&xLastWakeTime, 30 / portTICK_PERIOD_MS);
        TrackLen = (TrackSegment)status->TrackStatus->getData();
        pulsesBeforeCurve = latMarks->PulsesBeforeCurve->getData();
        pulsesAfterCurve = latMarks->PulsesAfterCurve->getData();
        actualCarState = (CarState)status->robotState->getData();

        if (status->robotPaused->getData())
            lastPaused = true;

        if (passedFirstMark())
            resetEnconderInFirstMark();

        if (trackSegmentChanged())
        {
            LedColor color = defineLedColor();
            setColorBrightness(color);
        }

        if (actualCarState == CAR_TUNING && !status->TunningMode->getData())
            stopTunningMode();

        if (actualCarState == CAR_ENC_READING && (!status->TunningMode->getData() || !started_in_Tuning))
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
                int mark = 0;
                for (mark = 0; mark < numMarks - 1; mark++)
                {
                    MapData currentMark = latMarks->marks->getData(mark);
                    MapData nextMark = latMarks->marks->getData(mark + 1);

                    if (robotPosition >= currentMark.markPosition && robotPosition <= nextMark.markPosition)
                    {
                        defineTrackSegment(nextMark);

                        bool transition = false;

                        int16_t offset = currentMark.offsetMarkPosition;
                        int16_t offsetnxt = nextMark.offsetMarkPosition;
                        // Verifica se o robô precisa reduzir a velocidade, entrando no modo curva
                        if (!isLineSegment((TrackSegment)currentMark.trackSegmentBeforeMark) && isLineSegment((TrackSegment)nextMark.trackSegmentBeforeMark) && offset == 0)
                        {
                            offset = pulsesAfterCurve;
                        }
                        if (offset > 0)
                        {
                            if (robotPosition < (currentMark.markPosition + offset))
                            {
                                transition = true;
                                trackLen = (TrackSegment)currentMark.trackSegmentBeforeMark;
                            }
                        }
                        if (mark + 2 < numMarks)
                        {

                            if (isLineSegment((TrackSegment)nextMark.trackSegmentBeforeMark) && !isLineSegment((TrackSegment)latMarks->marks->getData(mark + 2).trackSegmentBeforeMark) && offsetnxt == 0)
                            {
                                offsetnxt = -pulsesBeforeCurve;
                            }
                            if (offsetnxt < 0)
                            {
                                if (robotPosition > (nextMark.markPosition + offsetnxt))
                                {
                                    transition = true;
                                    trackLen = (TrackSegment)latMarks->marks->getData(mark + 2).trackSegmentBeforeMark;
                                }
                            }
                        }
                        // Atualiza estado do robô
                        status->Transition->setData(transition);
                        status->TrackStatus->setData(trackLen);
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
    DataStorage::getInstance()->delete_data("sLatMarks.marks");
    initialRobotState = CAR_MAPPING;
    ESP_LOGD(GetName().c_str(), "Mapeamento Deletado");
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_YELLOW, 1);
}

void CarStatusService::startMappingTheTrack()
{
    ESP_LOGD(GetName().c_str(), "Mapeamento inexistente, iniciando robô em modo mapemaneto.");
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_YELLOW, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // Começa mapeamento
    status->RealTrackStatus->setData(DEFAULT_TRACK);
    status->TrackStatus->setData(DEFAULT_TRACK);
    mappingService->startNewMapping();
}

void CarStatusService::setTuningMode()
{
    started_in_Tuning = true;
    initialRobotState = CAR_TUNING;
    latMarks->marks->clearAllData();
    numMarks = 0;
    LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_WHITE, 0.5);
}

bool CarStatusService::passedFirstMark()
{
    return latMarks->rightMarks->getData() >= 1 && actualCarState == CAR_ENC_READING_BEFORE_FIRSTMARK;
}

void CarStatusService::resetEnconderInFirstMark()
{
    SpeedService::getInstance()->resetEncondersValue();
    actualCarState = CAR_ENC_READING;
    status->robotState->setData(actualCarState);
}

bool CarStatusService::trackSegmentChanged()
{
    return lastTrack != (TrackSegment)status->TrackStatus->getData() || lastTransition != status->Transition->getData();
}

LedColor CarStatusService::defineLedColor()
{
    lastTrack = (TrackSegment)status->TrackStatus->getData();
    lastTransition = status->Transition->getData();
    LedColor color = getStatusColor((CarState)lastState, (TrackSegment)lastTrack);
    if (lastTransition)
        color = LED_COLOR_BLUE;
    return color;
}

void CarStatusService::setColorBrightness(LedColor color)
{
    float brightness = getSegmentBrightness((CarState)lastState, (TrackSegment)lastTrack);
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

void CarStatusService::defineTrackSegment(MapData nextMark)
{
    trackLen = (TrackSegment) nextMark.trackSegmentBeforeMark;
    status->RealTrackStatus->setData(trackLen);
}