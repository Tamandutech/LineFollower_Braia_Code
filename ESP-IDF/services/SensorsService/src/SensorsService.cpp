#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "SensorsService.h"

void SensorsService::Run()
{
    ESP_LOGD(GetName().c_str(), "Entrou no Run");
    // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Loop
    for (;;)
    {
        ESP_LOGD(GetName().c_str(), "Fez loop");
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

    // ESP_LOGD(Name.c_str(), "Array: %d | %d | %d | %d | %d | %d | %d | %d ", sArraychannels[0], sArraychannels[1], sArraychannels[2], sArraychannels[3], sArraychannels[4], sArraychannels[5], sArraychannels[6], sArraychannels[7]);
    // ESP_LOGD(Name.c_str(), "Linha: %d", robot->getsArray()->getLine());
    // ESP_LOGD(Name.c_str(), "Laterais: %d | %d ", SLatchannels[0], SLatchannels[1]);

    //robot->getsLat()->setLine((SLatchannels[0]+SLatchannels[1])/2-(SLatchannels[2]+SLatchannels[3])/2); // cálculo dos valores dos sensores laterais
}

void SensorsService::processSLat(Robot *robot)
{
    bool sldir1 = gpio_get_level(GPIO_NUM_17);
    bool sldir2 = gpio_get_level(GPIO_NUM_5);

    // ESP_LOGD(Name.c_str(), "Laterais (Direira): %d | %d", sldir1, sldir2);

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