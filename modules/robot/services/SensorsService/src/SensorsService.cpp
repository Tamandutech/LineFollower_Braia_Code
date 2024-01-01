#include "SensorsService.hpp"

SensorsService::SensorsService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();

    latMarks = robot->getSLatMarks();
    sLatData = robot->getsLat();
    status = robot->getStatus();
    

    // Definindo configs do ADC1 no GPIO36
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);

    // Definindo GPIOs e configs para sensor Array
    sArray.setTypeMCP3008();
    sArray.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, 8, (gpio_num_t)ADC_DOUT, (gpio_num_t)ADC_DIN, (gpio_num_t)ADC_CLK, (gpio_num_t)ADC_CS, 1350000, VSPI_HOST);
    sArray.setSamplesPerSensor(5);

    // Definindo GPIOs e configs para sensor Lateral
#ifndef ESP32_QEMU
    gpio_pad_select_gpio(39);
    gpio_set_direction(GPIO_NUM_17, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(05);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);
#endif

    sLat.setTypeAnalogESP();
    sLat.setSensorPins((const adc1_channel_t[]){(adc1_channel_t)SL1, (adc1_channel_t)SL2}, 2);
    sLat.setSamplesPerSensor(5);

    calibAllsensors();
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
        processSLat();
    }
}

void SensorsService::calibAllsensors()
{
    // Calibração dos sensores frontais
    command.led[0] = LED_POSITION_FRONT;
    command.led[1] = LED_POSITION_NONE;
    command.effect = LED_EFFECT_SET;
    command.brightness = 1;
    command.color = LED_COLOR_BLUE;
    LEDsService::getInstance()->queueCommand(command);
    for (uint16_t i = 0; i < 50; i++)
    {
        ESP_LOGD(GetName().c_str(), "(%p) | sArray: (%p)", this, &sArray);
        sArray.calibrate();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }


    // Calibração dos sensores laterais
    command.led[0] = LED_POSITION_FRONT;
    command.led[1] = LED_POSITION_NONE;
    command.effect = LED_EFFECT_SET;
    command.brightness = 1;
    command.color = LED_COLOR_RED;
    LEDsService::getInstance()->queueCommand(command); //mudar a cor
    for (uint16_t i = 0; i < 30; i++)
    {
        ESP_LOGD(GetName().c_str(), "(%p) | sLat: (%p)", this, &sLat);
        sLat.calibrate();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    // leitura e armazenamento dos valores máximos e mínimos dos sensores obtidos na calibração
    std::vector<uint16_t> sArrayMaxes(sArray.calibrationOn.maximum, sArray.calibrationOn.maximum + sArray.getSensorCount());
    std::vector<uint16_t> sArrayMins(sArray.calibrationOn.minimum, sArray.calibrationOn.minimum + sArray.getSensorCount());
    std::vector<uint16_t> SLatMaxes(sLat.calibrationOn.maximum, sLat.calibrationOn.maximum + sLat.getSensorCount());
    std::vector<uint16_t> SLatMins(sLat.calibrationOn.minimum, sLat.calibrationOn.minimum + sLat.getSensorCount());

    // armazenamento dos valores máximos e mínimos dos sensores no objeto robot
    robot->getsArray()->setChannelsMaxes(sArrayMaxes);
    robot->getsArray()->setChannelsMins(sArrayMins);
    robot->getsLat()->setChannelsMaxes(SLatMaxes);
    robot->getsLat()->setChannelsMins(SLatMins);

    ESP_LOGD(GetName().c_str(), "------------");
    ESP_LOGD(GetName().c_str(), "sArrayMaxes: %d | %d | %d | %d | %d | %d | %d | %d", sArrayMaxes[0], sArrayMaxes[1], sArrayMaxes[2], sArrayMaxes[3], sArrayMaxes[4], sArrayMaxes[5], sArrayMaxes[6], sArrayMaxes[7]);
    ESP_LOGD(GetName().c_str(), "sArrayMins: %d | %d | %d | %d | %d | %d | %d | %d", sArrayMins[0], sArrayMins[1], sArrayMins[2], sArrayMins[3], sArrayMins[4], sArrayMins[5], sArrayMins[6], sArrayMins[7]);
    ESP_LOGD(GetName().c_str(), "------------");
    ESP_LOGD(GetName().c_str(), "SLatMaxes: %d | %d", SLatMaxes[0], SLatMaxes[1]);
    ESP_LOGD(GetName().c_str(), "SLatMins: %d | %d", SLatMins[0], SLatMins[1]);
    ESP_LOGD(GetName().c_str(), "------------");

    command.led[0] = LED_POSITION_FRONT;
    command.led[1] = LED_POSITION_NONE;
    command.effect = LED_EFFECT_SET;
    command.brightness = 1;
    command.color = LED_COLOR_BLACK;
    LEDsService::getInstance()->queueCommand(command);
}

void SensorsService::getLatSensors() // função leitura dos sensores
{
    // Arrays para armazenar leitura bruta dos sensores laterais
    uint16_t SLatchannels[sLat.getSensorCount()];

    sLat.readCalibrated(SLatchannels);                                                                 // leitura dos sensores laterais
    std::vector<uint16_t> SLatchannelsVec(SLatchannels, SLatchannels + sLat.getSensorCount());         // vector(array) com os valores dos sensores laterais

    // armazenando da leitura bruta do sensor lateral no objeto Braia
    robot->getsLat()->setChannels(SLatchannelsVec);

    if (latloop >= 100)
    {
        ESP_LOGD(GetName().c_str(), "Laterais -  Esqurdo: %d | Direito : %d ", robot->getsLat()->getChannel(0), robot->getsLat()->getChannel(1));
        latloop = 0;
    }
    latloop++;
}

uint16_t SensorsService::getArraySensors() // função leitura dos sensores frontais
{
    // Arrays para armazenar leitura bruta dos sensores array
    uint16_t sArraychannels[sArray.getSensorCount()];

    if(status->LineColorBlack->getData()) robot->getsArray()->setLine(sArray.readLineBlack(sArraychannels));
    else robot->getsArray()->setLine(sArray.readLineWhite(sArraychannels));
    // cálculo dos valores do sensor array
    std::vector<uint16_t> sArraychannelsVec(sArraychannels, sArraychannels + sArray.getSensorCount()); // vector(array) com os valores do sensor array       // vector(array) com os valores dos sensores laterais

    // armazenando da leitura bruta do sensor array no objeto Braia
    robot->getsArray()->setChannels(sArraychannelsVec);

    if (sloop >= 100)
    {
        ESP_LOGD(GetName().c_str(), "Array: %d | %d | %d | %d | %d | %d | %d | %d ", sArraychannels[0], sArraychannels[1], sArraychannels[2], sArraychannels[3], sArraychannels[4], sArraychannels[5], sArraychannels[6], sArraychannels[7]);
        ESP_LOGD(GetName().c_str(), "Linha: %d", robot->getsArray()->getLine());
        sloop = 0;
    }
    sloop++;

    return robot->getsArray()->getLine();
}

void SensorsService::processSLat()
{
    uint16_t slesq = sLatData->getChannel(0);
    uint16_t sldir = sLatData->getChannel(1);
    
    nLatReads++; 
    sumSensEsq += slesq;
    sumSensDir += sldir;

    if(status->robotIsMapping->getData())
    {
        MarksToMean = latMarks->MarkstoMean->getData();
    }
    else
    {
        MarksToMean = 1;
    }

    if (nLatReads >= MarksToMean)  //valor definido na dashboard
    {
        int meanSensDir = (sumSensDir/nLatReads);
        int meanSensEsq = (sumSensEsq/nLatReads);

        if (meanSensEsq < 300 || meanSensDir < 300) // leitura de faixas brancas sensores laterais
        {
            if ((meanSensEsq < 300) && (meanSensDir > 600)) // lendo sLat esq. branco e dir. preto
            {
                if (!(latMarks->latEsqPass->getData()))
                {
                    if(status->robotState->getData() != CAR_STOPPED)
                    {
                        latMarks->leftPassedInc();
                    }

                    latMarks->latEsqPass->setData(true);
                    latMarks->latDirPass->setData(false);
                    
                    command.effect = LED_EFFECT_SET;
                    command.brightness = 1;
                    command.led[1] = LED_POSITION_NONE;
                    command.led[0] = LED_POSITION_LEFT;
                    command.color = LED_COLOR_RED;
                    LEDsService::getInstance()->queueCommand(command);
                    command.led[0] = LED_POSITION_RIGHT;
                    command.color = LED_COLOR_BLACK;
                    LEDsService::getInstance()->queueCommand(command);
                }
            }
            else if ((meanSensDir < 300) && (meanSensEsq > 600)) // lendo sldir. branco e sLat esq. preto
            {
                if (!(latMarks->latDirPass->getData()))
                {
                    if(status->robotState->getData() != CAR_STOPPED)
                    {
                        latMarks->rightPassedInc();

                    }

                    latMarks->latDirPass->setData(true);
                    latMarks->latEsqPass->setData(false);
                    command.effect = LED_EFFECT_SET;
                    command.brightness = 1;

                    command.led[1] = LED_POSITION_NONE;

                    command.led[0] = LED_POSITION_RIGHT;
                    command.color = LED_COLOR_RED;
                    LEDsService::getInstance()->queueCommand(command);

                    command.led[0] = LED_POSITION_LEFT;
                    command.color = LED_COLOR_BLACK;
                    LEDsService::getInstance()->queueCommand(command);
                }
            }

            else if ((meanSensEsq < 300) && (meanSensDir < 300)) // quando ler ambos brancos, contar nova marcação apenas se ambos os sensores lerem preto antes de lerem a nova marcação 
            {
                if ((latMarks->latDirPass->getData() && !latMarks->latEsqPass->getData()) 
                    || (latMarks->latEsqPass->getData() && !latMarks->latDirPass->getData()))
                { 
                    command.effect = LED_EFFECT_SET;
                    command.brightness = 1;
                    command.led[1] = LED_POSITION_RIGHT;
                    command.led[0] = LED_POSITION_LEFT;
                    command.led[2] = LED_POSITION_NONE;
                    command.color = LED_COLOR_BLACK;
                    LEDsService::getInstance()->queueCommand(command);
                }
                latMarks->latDirPass->setData(true);
                latMarks->latEsqPass->setData(true);
            }
        }
        else
        {
            if (latMarks->latDirPass->getData() || latMarks->latEsqPass->getData())
            {
                command.effect = LED_EFFECT_SET;
                command.brightness = 1;
                command.led[1] = LED_POSITION_RIGHT;
                command.led[0] = LED_POSITION_LEFT;
                command.led[2] = LED_POSITION_NONE;
                command.color = LED_COLOR_BLACK;
                LEDsService::getInstance()->queueCommand(command);
            }

            latMarks->latDirPass->setData(false);
            latMarks->latEsqPass->setData(false);
        }
        nLatReads = 0;
        sumSensDir = 0;
        sumSensEsq = 0;
    }
}