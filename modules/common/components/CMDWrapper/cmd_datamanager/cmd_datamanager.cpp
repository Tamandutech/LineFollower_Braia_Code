#include "cmd_datamanager.hpp"

#include "DataManager.hpp"
#include "dataEnums.h"

#include "esp_log.h"

#include "better_console.hpp"
#include "argtable3/argtable3.h"

#include "RobotData.h"

static const char *name = "CMD_DATAMANAGER";

void register_cmd_datamanager(void)
{
    register_start_stream();
}

static struct
{
    struct arg_str *name;
    struct arg_str *interval;
    struct arg_end *end;
} start_stream_args;

static std::string start_stream(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&start_stream_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, start_stream_args.end, argv[0]);
        return "NOK";
    }
    ESP_LOGD(name, "Iniciando stream do parâmetro local %s com intervalo %s", start_stream_args.name->sval[0], start_stream_args.interval->sval[0]);
    DataManager::getInstance()->setStreamInterval(start_stream_args.name->sval[0],std::stoi(start_stream_args.interval->sval[0]));
    return "OK";
}

void register_start_stream(void)
{
    start_stream_args.name = arg_str1(NULL, NULL, "<nome>", "nome da variável para stream.");
    start_stream_args.interval = arg_str1(NULL, NULL, "<intervalo>", "intevalo de atualização em ms.");
    start_stream_args.end = arg_end(2);

    const better_console_cmd_t start_stream_cmd = {
        .command = "start_stream",
        .help = "Inicia o stream de um determinado dado do robô.",
        .hint = NULL,
        .func = &start_stream,
        .argtable = &start_stream_args};

    ESP_ERROR_CHECK(better_console_cmd_register(&start_stream_cmd));
}