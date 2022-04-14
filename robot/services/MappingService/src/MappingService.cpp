#include "MappingService.hpp"

MappingService::MappingService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;

#ifndef ESP32_QEMU
    gpio_pad_select_gpio(0);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
#endif

    ESP_LOGD(GetName().c_str(), "Mapeamento Iniciado e aguardando calibração");
    ESP_LOGD(GetName().c_str(), "Mapeamento Retomado!");

    speedMapping = robot->getSpeed();
    SLat = robot->getsLat();
    latMarks = robot->getSLatMarks();
    status = robot->getStatus();

    markreg.MapEncMedia = 0;
    markreg.MapEncLeft = 0;
    markreg.MapEncRight = 0;
    markreg.MapTime = 0;
    markreg.MapStatus = CAR_IN_LINE;

    latMarks->SetMarkDataReg(markreg, 0);
};

void MappingService::Run()
{

    TickType_t xLastWakeTime = xTaskGetTickCount(); // Variavel necerraria para funcionalidade do vTaskDelayUtil, quarda a conGetName().c_str()em de pulsos da CPU
    TickType_t xInicialTicks = xTaskGetTickCount();

    // Loop
    for (;;)
    {
        CarState Parar = (CarState) status->robotState->getData(); // Verifica se o mapeamento deve iniciar
        bool mapping = status->robotMap->getData(); // Verifica se o mapeamento deve iniciar
#if defined(BRAIA_V2)        
        uint16_t slesq1 = SLat->getChannel(0);
        uint16_t slesq2 = SLat->getChannel(1);
#elif defined(BRAIA_V3)
        uint16_t slesq = SLat->getChannel(0);
        uint16_t sldir = SLat->getChannel(1);
#else
        uint16_t slesq1 = SLat->getChannel(0);
        uint16_t slesq2 = SLat->getChannel(1);
#endif

#ifdef ESP32_QEMU
        // bool sldir1 = gpio_get_level(GPIO_NUM_17);
        bool sldir2 = false;
        bool bottom = false;
#else
        // bool sldir1 = gpio_get_level(GPIO_NUM_17);
        bool sldir2 = gpio_get_level(GPIO_NUM_5);
        bool bottom = gpio_get_level(GPIO_NUM_0);
#endif

        mapfinish = latMarks->mapFinished->getData();

        if (((latMarks->rightMarks->getData()) == 1) && !startTimer && Parar != CAR_STOPPED && mapping)
        {

            xInicialTicks = xTaskGetTickCount(); // pegando o tempo inicial
            startTimer = true;
            InitialMarkData = ((speedMapping->EncRight->getData()) + (speedMapping->EncLeft->getData())) / 2;
            latMarks->initialMark->setData(InitialMarkData);
        }

#if LOG_LOCAL_LEVEL >= ESP_LOG_ERROR
        if (iloop >= 20 && mapping)
        {
#if defined(BRAIA_V2)
            ESP_LOGD(GetName().c_str(), "Laterais Esquerdos: %d | %d ", slesq1, slesq2);
            ESP_LOGD(GetName().c_str(), "Laterais Direitos: %d", sldir2);
#elif defined(BRAIA_V3)
            ESP_LOGD(GetName().c_str(), "Laterais Esquerdos: %d", slesq);
            ESP_LOGD(GetName().c_str(), "Laterais Direitos: %d", sldir);
#else
            ESP_LOGD(GetName().c_str(), "Laterais Esquerdos: %d | %d ", slesq1, slesq2);
            ESP_LOGD(GetName().c_str(), "Laterais Direitos: %d", sldir2);
#endif
            ESP_LOGD(GetName().c_str(), "Marcações direita: %d ", latMarks->rightMarks->getData());
            iloop = 0;
        }
        iloop++;
#endif

        if ((latMarks->rightMarks->getData()) == 1 && mapping && Parar != CAR_STOPPED)
        {
#if defined(BRAIA_V2)
            if ((slesq1 < 300) && (sldir2) && !leftpassed)
#elif defined(BRAIA_V3)
            if ((slesq < 300) && (sldir>600) && !leftpassed)
#else
            if ((slesq1 < 300) && (sldir2) && !leftpassed)
#endif
            {
                struct MapData MarkReg;
                // tempo
                MarkReg.MapTime = (xTaskGetTickCount() - xInicialTicks) * portTICK_PERIOD_MS;
                // Contagem encoder esquerdo
                MarkReg.MapEncLeft = speedMapping->EncLeft->getData();
                // Contagem encoder direito
                MarkReg.MapEncLeft = speedMapping->EncRight->getData();
                // media
                MarkReg.MapEncMedia = ((speedMapping->EncRight->getData()) + (speedMapping->EncLeft->getData())) / 2;
                // estado
#if defined(ManualMap)
                if ((marks % 2) == 0)
#elif defined(AutoMap)
                if(abs((speedMapping->getEncRight()) - (speedMapping->getEncLeft())) >= 240)  
#endif
                { // Verifica se o carrinho está na curva ou na linha
                    MarkReg.MapStatus = CAR_IN_CURVE;
                }
                else
                {
                    MarkReg.MapStatus = CAR_IN_LINE;
                }
                ESP_LOGI(GetName().c_str(), "%d, %d, %d", MarkReg.MapTime, MarkReg.MapEncMedia, MarkReg.MapStatus);
                latMarks->SetMarkDataReg(MarkReg, marks + 1); // Salva os dados da marcação na struct MapData

                // struct PacketData markData;
                // markData.cmd = MarkData;
                // markData.version = 1;
                // markData.size = sizeof(MarkReg);
                // memcpy(markData.data, &MarkReg, sizeof(MarkReg));
                // robot->addPacketSend(markData);
                
                marks++;
                leftpassed = true; // Diz que o carro está em uma marcação esquerda
            }
#if defined(BRAIA_V2)
            else if (slesq1 > 600)
#elif defined(BRAIA_V3)
            else if (slesq > 600)
#else
            else if (slesq1 > 600)
#endif
            {
                leftpassed = false;
            }
        }
        else if (latMarks->rightMarks->getData() < 1 && mapping && Parar != CAR_STOPPED)
        {
            // ESP_LOGI(GetName().c_str(), "Mapeamento não iniciado");
        }
        else if (latMarks->rightMarks->getData() > 1 && !mapfinish && mapping)
        {
            // ESP_LOGI(GetName().c_str(), "Mapeamento finalizado");
            FinalMarkData = ((speedMapping->EncRight->getData()) + (speedMapping->EncLeft->getData())) / 2;
            latMarks->mapFinished->setData(true);
            latMarks->finalMark->setData(FinalMarkData);
            latMarks->totalLeftMarks->setData(marks);
            
            // struct PacketData mapPacket;
            // mapPacket.cmd = MapDataSend;
            // mapPacket.version = 1;
            // mapPacket.size = sizeof(struct SLatMarks);
            // robot->addPacketSend(mapPacket);
        }
        if (!bottom && latMarks->mapFinished->getData())
        {
            // struct PacketData mapPacket;
            // mapPacket.cmd = MapDataSend;
            // mapPacket.version = 1;
            // mapPacket.size = sizeof(struct SLatMarks);
            // robot->addPacketSend(mapPacket);

            ESP_LOGI("", "Tempo, Média, Estado");
            for (int i = 0; i < marks + 1; i++)
            {
                struct MapData markreg = latMarks->getMarkDataReg(i);
                ESP_LOGI("", "%d, %d, %d", markreg.MapTime, markreg.MapEncMedia, markreg.MapStatus);
            }
            ESP_LOGI("Initial Mark (média dos encoders) ", " %d ", latMarks->initialMark->getData());
            ESP_LOGI("Final Mark (média dos encoders) ", " %d ", latMarks->finalMark->getData());
            ESP_LOGI("Total de marcações esquerdas ", " %d ", latMarks->totalLeftMarks->getData());
        }

        vTaskDelayUntil(&xLastWakeTime, 30 / portTICK_PERIOD_MS);
    }
}