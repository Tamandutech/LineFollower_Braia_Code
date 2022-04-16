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

        for (uint8_t i = 0; i < 50; i++)
        {
            packet = ESPNOWHandler::getInstance()->getPacketReceived(uniqueId);

            if (packet.size > 0)
            {
                ESP_LOGD(name, "Recebido: %s", (const char *)packet.data);
                return "OK";
            }

            ESP_LOGD(name, "Aguardando resposta do robô...");
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        ESP_LOGE(name, "Não recebido nenhum pacote de retorno.");
    }

    return "NOK: Sem resposta.";
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
