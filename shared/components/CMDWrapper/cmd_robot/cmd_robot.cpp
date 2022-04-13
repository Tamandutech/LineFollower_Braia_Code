#include "cmd_robot.hpp"

#include "EspNowHandler.h"
#include "dataEnums.h"
#include "esp_log.h"

#include "better_console.hpp"
#include "argtable3/argtable3.h"

static const char *name = "CMD_ROBOT";

void register_cmd_robot(void)
{
    register_param_set();
}

static struct
{
    struct arg_str *name;
    struct arg_str *value;
    struct arg_end *end;
} param_set_args;

static int param_set(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&param_set_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, param_set_args.end, argv[0]);
        return 1;
    }

    ESP_LOGD(name, "Setting parameter %s to %s", param_set_args.name->sval[0], param_set_args.value->sval[0]);

    struct ParamData ParamsSend;

    memcpy(ParamsSend.value, param_set_args.value->sval[0], sizeof(ParamsSend.value));
    memcpy(ParamsSend.name, param_set_args.name->sval[0], sizeof(ParamsSend.name));
    ParamsSend.cmdType[0] = 'S';

    ESP_LOGD(name, "Parametro: %s com valor: %s e comando: %s", ParamsSend.name, ParamsSend.value, ParamsSend.cmdType);

    struct PacketData packetDataSendcmd;
    packetDataSendcmd.cmd = ParametersSend;
    packetDataSendcmd.version = 1;
    packetDataSendcmd.size = sizeof(ParamsSend);
    memcpy(&packetDataSendcmd.data, &ParamsSend, packetDataSendcmd.size);

    EspNowHandler::getInstance()->EspSend(packetDataSendcmd.cmd, packetDataSendcmd.version, packetDataSendcmd.size, &packetDataSendcmd.data);

    return 0;
}

void register_param_set(void)
{
    param_set_args.name = arg_str1(NULL, NULL, "<Parametro>", "Nome do parâmetro a ser alterado no robô.");
    param_set_args.value = arg_str1(NULL, NULL, "<Valor>", "Valor a ser atribuido ao parametro.");
    param_set_args.end = arg_end(2);

    const better_console_cmd_t param_set_cmd = {
        .command = "param_set",
        .help = "Altera o valor de um parametro no robô.",
        .hint = NULL,
        .func = &param_set,
        .argtable = &param_set_args};

    better_console_cmd_register(&param_set_cmd);
}