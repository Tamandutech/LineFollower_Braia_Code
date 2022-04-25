#include "cmd_remote.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "ESPNOWHandler.h"
#include "DataManager.hpp"
#include "dataEnums.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#include "better_console.hpp"
#include "argtable3/argtable3.h"

static const char *name = "CMD_REMOTE";

void register_cmd_remote(void)
{
    register_rmt();
}

static struct
{
    struct arg_str *command;
    struct arg_lit *wait_return;
    struct arg_end *end;
} rmt_args;

static std::string rmt(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&rmt_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, rmt_args.end, argv[0]);
        return "NOK";
    }

    ESP_LOGD(name, "Executando comando remoto %s", rmt_args.command->sval[0]);

    uint8_t uniqueId = ESPNOWHandler::getInstance()->SendCMD((uint8_t *)rmt_args.command->sval[0], strlen(rmt_args.command->sval[0]) + 1);

    if (rmt_args.wait_return->count > 0)
    {
        PacketData packet;
        packet.size = 0;

        uint8_t numActual, numToReceive;

        char *bufferAwnser = NULL;

        for (numActual = 1, numToReceive = 1; numActual <= numToReceive; numActual++)
        {
            for (uint8_t i = 0; i < 50; i++)
            {
                packet = ESPNOWHandler::getInstance()->getPacketReceived(uniqueId, numActual);

                if (packet.size > 0 and packet.numActual > 0)
                {
                    numToReceive = packet.numToReceive;

                    ESP_LOGD(name, "Recebido pacote ID: %d | %d de %d.", packet.id, packet.numActual, packet.numToReceive);

                    if (bufferAwnser == NULL)
                        bufferAwnser = (char *)malloc(sizeof(packet.data) * packet.numToReceive);

                    memcpy(bufferAwnser + (packet.numActual - 1) * sizeof(packet.data), packet.data, sizeof(packet.data));

                    break;
                }

                ESP_LOGD(name, "Aguardando resposta %d do robô...", numActual);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }

        printf("%s\n", bufferAwnser);

        free(bufferAwnser);

        if (packet.size == 0)
            return "Sem resposta.";
        else
        {
            if ((numActual - 1) == numToReceive)
                return "Resposta recebida com sucesso.";
            else
                return "Resposta incompleta, %d de %d pacotes recebidos.";
        }
    }
    else
    {
        return "Comando enviado.";
    }
}

void register_rmt(void)
{
    rmt_args.wait_return = arg_lit0("w", "wait_return", "Espera o retorno do comando.");
    rmt_args.command = arg_str1(NULL, NULL, "<comando>", "Comando a ser executado.");
    rmt_args.end = arg_end(2);

    const better_console_cmd_t rmt_cmd = {
        .command = "rmt",
        .help = "Executa um comando remoto.",
        .hint = NULL,
        .func = &rmt,
        .argtable = &rmt_args};

    better_console_cmd_register(&rmt_cmd);
}
