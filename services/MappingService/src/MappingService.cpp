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
        CarState Parar = status->robotState->getData(); // Verifica se o mapeamento deve iniciar
        bool mapping = status->robotMap->getData(); // Verifica se o mapeamento deve iniciar
        uint16_t slesq1 = SLat->getChannel(0);
        uint16_t slesq2 = SLat->getChannel(1);

#ifdef ESP32_QEMU
        // bool sldir1 = gpio_get_level(GPIO_NUM_17);
        bool sldir2 = false;
        bool bottom = false;
#else
        // bool sldir1 = gpio_get_level(GPIO_NUM_17);
        bool sldir2 = gpio_get_level(GPIO_NUM_5);
        bool bottom = gpio_get_level(GPIO_NUM_0);
#endif

        mapfinish = latMarks->getMapFinished();

        if (((latMarks->getrightMarks()) == 1) && !startTimer && Parar != CAR_STOPPED && mapping)
        {

            xInicialTicks = xTaskGetTickCount(); // pegando o tempo inicial
            startTimer = true;
            InitialMarkData = ((speedMapping->getEncRight()) + (speedMapping->getEncLeft())) / 2;
            latMarks->SetInitialMark(InitialMarkData);
        }

#if LOG_LOCAL_LEVEL >= ESP_LOG_ERROR
        if (iloop >= 20 && mapping)
        {
            ESP_LOGD(GetName().c_str(), "Laterais Esquerdos: %d | %d ", slesq1, slesq2);
            ESP_LOGD(GetName().c_str(), "Laterais Direitos: %d", sldir2);
            ESP_LOGD(GetName().c_str(), "Marcações direita: %d ", latMarks->getrightMarks());
            iloop = 0;
        }
        iloop++;
#endif

        if ((latMarks->getrightMarks()) == 1 && mapping && Parar != CAR_STOPPED)
        {
            if ((slesq1 < 300) && (sldir2) && !leftpassed)
            {
                struct MapData MarkReg;
                // tempo
                MarkReg.MapTime = (xTaskGetTickCount() - xInicialTicks) * portTICK_PERIOD_MS;
                // media
                MarkReg.MapEncMedia = ((speedMapping->getEncRight()) + (speedMapping->getEncLeft())) / 2;
                // estado
                if ((marks % 2) == 0)
                { // Verifica se o carrinho está na curva ou na linha
                    MarkReg.MapStatus = CAR_IN_CURVE;
                }
                else
                {
                    MarkReg.MapStatus = CAR_IN_LINE;
                }
                ESP_LOGI(GetName().c_str(), "%d, %d, %d", MarkReg.MapTime, MarkReg.MapEncMedia, MarkReg.MapStatus);
                latMarks->SetMarkDataReg(MarkReg, marks + 1); // Salva os dados da marcação na struct MapData
                struct PacketData markData;
                markData.cmd = MarkData;
                markData.version = 1;
                markData.size = sizeof(MarkReg);
                memcpy(markData.data, &MarkReg, sizeof(MarkReg));
                robot->addPacketSend(markData);
                marks++;
                leftpassed = true; // Diz que o carro está em uma marcação esquerda
            }
            else if (slesq1 > 600)
            {
                leftpassed = false;
            }
        }
        else if (latMarks->getrightMarks() < 1 && mapping && Parar != CAR_STOPPED)
        {
            // ESP_LOGI(GetName().c_str(), "Mapeamento não iniciado");
        }
        else if (latMarks->getrightMarks() > 1 && !mapfinish && mapping)
        {
            // ESP_LOGI(GetName().c_str(), "Mapeamento finalizado");
            FinalMarkData = ((speedMapping->getEncRight()) + (speedMapping->getEncLeft())) / 2;
            latMarks->SetMapFinished(true);
            latMarks->SetFinalMark(FinalMarkData);
            latMarks->SetTotalLeftMarks(marks);
            struct PacketData mapPacket;
            mapPacket.cmd = MapDataSend;
            mapPacket.version = 1;
            mapPacket.size = sizeof(struct SLatMarks);
            robot->addPacketSend(mapPacket);
        }
        if (!bottom && latMarks->getMapFinished())
        {
            struct PacketData mapPacket;
            mapPacket.cmd = MapDataSend;
            mapPacket.version = 1;
            mapPacket.size = sizeof(struct SLatMarks);
            robot->addPacketSend(mapPacket);
            ESP_LOGI("", "Tempo, Média, Estado");
            for (int i = 0; i < marks + 1; i++)
            {
                struct MapData markreg = latMarks->getMarkDataReg(i);
                ESP_LOGI("", "%d, %d, %d", markreg.MapTime, markreg.MapEncMedia, markreg.MapStatus);
            }
            ESP_LOGI("Initial Mark (média dos encoders) ", " %d ", latMarks->getInitialMark());
            ESP_LOGI("Final Mark (média dos encoders) ", " %d ", latMarks->getFinalMark());
            ESP_LOGI("Total de marcações esquerdas ", " %d ", latMarks->getTotalLeftMarks());
        }

        vTaskDelayUntil(&xLastWakeTime, 30 / portTICK_PERIOD_MS);
    }
}