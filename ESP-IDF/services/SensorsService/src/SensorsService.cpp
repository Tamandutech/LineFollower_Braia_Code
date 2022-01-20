#include "SensorsService.h"

void SensorsService::Setup()
{
    // Definindo GPIOs e configs para sensor Array
    sArray.setTypeMCP3008();
    sArray.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, 8, (gpio_num_t)ADC_DOUT, (gpio_num_t)ADC_DIN, (gpio_num_t)ADC_CLK, (gpio_num_t)ADC_CS, 1350000, VSPI_HOST);
    sArray.setSamplesPerSensor(5);

    // Definindo GPIOs e configs para sensor Lateral
    gpio_pad_select_gpio(39);
    gpio_set_direction(GPIO_NUM_17, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(05);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);

    sLat.setTypeAnalogESP();
    sLat.setSensorPins((const adc1_channel_t[]){SL1, SL2}, 2);
    sLat.setSamplesPerSensor(5);

    //D (470)  SensorsService: (0x3ffb5f10) | sArray: (0x3ffb5f30) | sLat: (0x3ffb5f94)
    //D (1560) SensorsService: (0x3ffb5f10) | sArray: (0x3ffb5f30) | sLat: (0x3ffb5f94)

    //Calibração dos dos sensores laterais e array
    for (uint16_t i = 0; i < 20; i++)
    {
        ESP_LOGD(name, "(%p) | sArray: (%p) | sLat: (%p)", this, &sArray, &sLat);
        sArray.calibrate();
        sLat.calibrate();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    // //leitura e armazenamento dos valores máximos e mínimos dos sensores obtidos na calibração
    // std::vector<uint16_t> sArrayMaxes(sArray.calibrationOn.maximum, sArray.calibrationOn.maximum + sArray.getSensorCount());
    // std::vector<uint16_t> sArrayMins(sArray.calibrationOn.minimum, sArray.calibrationOn.minimum + sArray.getSensorCount());
    // std::vector<uint16_t> SLatMaxes(sLat.calibrationOn.maximum, sLat.calibrationOn.maximum + sLat.getSensorCount());
    // std::vector<uint16_t> SLatMins(sLat.calibrationOn.minimum, sLat.calibrationOn.minimum + sLat.getSensorCount());

    // //armazenamento dos valores máximos e mínimos dos sensores no objeto robot
    // robot->getsArray()->setChannelsMaxes(sArrayMaxes);
    // robot->getsArray()->setChannelsMins(sArrayMins);
    // robot->getsLat()->setChannelsMaxes(SLatMaxes);
    // robot->getsLat()->setChannelsMins(SLatMins);

    // vTaskResume(xTaskMotors);
    // vTaskResume(xTaskPID);
    // if (taskStatus)
    //     vTaskResume(xTaskCarStatus);
    // vTaskResume(xTaskSpeed);
    // if (robot->getStatus()->getMapping())
    //     vTaskResume(xTaskMapping);
}

void SensorsService::Main()
{
    // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Loop
    for (;;)
    {
        getSensors(&sArray, &sLat, robot); // leitura dos sensores
        processSLat(robot);

        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
    }
}

void SensorsService::getSensors(QTRSensors *sArray, QTRSensors *SLat, Robot *robot) // função leitura dos sensores
{
    //Arrays para armazenar leitura bruta dos sensores array e laterais
    uint16_t sArraychannels[sArray->getSensorCount()];
    uint16_t SLatchannels[SLat->getSensorCount()];

#ifdef LINE_COLOR_BLACK
    robot->getsArray()->setLine(sArray->readLineBlack(sArraychannels));
#else
    robot->getsArray()->setLine(sArray->readLineWhite(sArraychannels));
#endif
    // cálculo dos valores do sensor array
    SLat->readCalibrated(SLatchannels);                                                                 //leitura dos sensores laterais
    std::vector<uint16_t> sArraychannelsVec(sArraychannels, sArraychannels + sArray->getSensorCount()); // vector(array) com os valores do sensor array
    std::vector<uint16_t> SLatchannelsVec(SLatchannels, SLatchannels + SLat->getSensorCount());         // vector(array) com os valores dos sensores laterais

    //armazenando da leitura bruta do sensor array e lateral no objeto robot
    robot->getsArray()->setChannels(sArraychannelsVec);
    robot->getsLat()->setChannels(SLatchannelsVec);

    ESP_LOGD(name, "Array: %d | %d | %d | %d | %d | %d | %d | %d ", sArraychannels[0], sArraychannels[1], sArraychannels[2], sArraychannels[3], sArraychannels[4], sArraychannels[5], sArraychannels[6], sArraychannels[7]);
    ESP_LOGD(name, "Linha: %d", robot->getsArray()->getLine());
    ESP_LOGD(name, "Laterais: %d | %d ", SLatchannels[0], SLatchannels[1]);

    //robot->getsLat()->setLine((SLatchannels[0]+SLatchannels[1])/2-(SLatchannels[2]+SLatchannels[3])/2); // cálculo dos valores dos sensores laterais
}

void SensorsService::processSLat(Robot *robot)
{
    bool sldir1 = gpio_get_level(GPIO_NUM_17);
    bool sldir2 = gpio_get_level(GPIO_NUM_5);

    ESP_LOGD(name, "Laterais (Direira): %d | %d", sldir1, sldir2);

    auto SLat = robot->getsLat();
    uint16_t slesq1 = SLat->getChannel(0);
    uint16_t slesq2 = SLat->getChannel(1);
    auto latMarks = robot->getSLatMarks();
    if (slesq1 < 300 || slesq2 < 300 || !sldir1 || !sldir2) // leitura de faixas brancas sensores laterais
    {
        if ((slesq1 < 300 || slesq2 < 300) && (sldir1 && sldir2)) //lendo sLat esq. branco e dir. preto
        {
            if (!(latMarks->getSLatEsq()))
                latMarks->leftPassedInc();
            latMarks->SetSLatEsq(true);
            latMarks->SetSLatDir(false);
        }
        else if ((!sldir1 || !sldir2) && (slesq1 > 600 && slesq2 > 600)) // lendo sldir. branco e sLat esq. preto
        {
            if (!(latMarks->getSLatDir()))
                latMarks->rightPassedInc();
            latMarks->SetSLatDir(true);
            latMarks->SetSLatEsq(false);
        }
    }
    else
    {
        latMarks->SetSLatDir(false);
        latMarks->SetSLatEsq(false);
    }

    if (slesq1 < 300 && slesq2 < 300 && !sldir1 && !sldir2)
    { //continuar em frente em intersecção de linhas
        robot->getStatus()->setState(CAR_IN_LINE);
    }

    if (latMarks->getrightMarks() >= 2)
    { //parar depois da leitura da segunda linha direita
        vTaskDelay(500);
        robot->getStatus()->setState(CAR_STOPPED);
    }
}