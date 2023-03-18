#include "cmd_runtime.hpp"

#include "DataManager.hpp"
#include "dataEnums.h"

#include "esp_log.h"

#include "better_console.hpp"
#include "argtable3/argtable3.h"

#include "RobotData.h"

static const char *name = "CMD_RUNTIME";

void register_cmd_runtime(void)
{
    register_runtime_set();
    register_runtime_get();
    register_runtime_list();
}

static struct
{
    struct arg_str *name;
    struct arg_str *value;
    struct arg_end *end;
} runtime_set_args;

static std::string runtime_set(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&runtime_set_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, runtime_set_args.end, argv[0]);
        return "NOK";
    }

    ESP_LOGD(name, "Definido runtime data local %s com valor %s", runtime_set_args.name->sval[0], runtime_set_args.value->sval[0]);
    DataManager::getInstance()->setRuntime(runtime_set_args.name->sval[0], runtime_set_args.value->sval[0]);
    return "OK";
}

void register_runtime_set(void)
{
    runtime_set_args.name = arg_str1(NULL, NULL, "<runtime>", "Nome do runtime data a ser alterado no robô.");
    runtime_set_args.value = arg_str1(NULL, NULL, "<valor>", "Valor a ser atribuido ao runtime data.");
    runtime_set_args.end = arg_end(2);

    const better_console_cmd_t runtime_set_cmd = {
        .command = "runtime_set",
        .help = "Altera o valor de um runtime data.",
        .hint = NULL,
        .func = &runtime_set,
        .argtable = &runtime_set_args};

    ESP_ERROR_CHECK(better_console_cmd_register(&runtime_set_cmd));
}

static struct
{
    struct arg_str *name;
    struct arg_end *end;
} runtime_get_args;

static std::string runtime_get(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&runtime_get_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, runtime_get_args.end, argv[0]);
        return "NOK";
    }

    ESP_LOGD(name, "Buscando runtime data local %s", runtime_get_args.name->sval[0]);

    return DataManager::getInstance()->getRuntime(runtime_get_args.name->sval[0]);
}

void register_runtime_get(void)
{
    runtime_get_args.name = arg_str1(NULL, NULL, "<runtime>", "Nome do runtime data a ser alterado no robô.");
    runtime_get_args.end = arg_end(2);

    const better_console_cmd_t runtime_get_cmd = {
        .command = "runtime_get",
        .help = "Busca o valor de um runtime data.",
        .hint = NULL,
        .func = &runtime_get,
        .argtable = &runtime_get_args};

    ESP_ERROR_CHECK(better_console_cmd_register(&runtime_get_cmd));
}

static std::string runtime_list(int argc, char **argv)
{
    std::string ret = DataManager::getInstance()->listRegistredRuntimeData();
    ESP_LOGD(name, "Tamanho da lista: %d", ret.length());
    return ret;
}

void register_runtime_list(void)
{
    const better_console_cmd_t runtime_list_cmd = {
        .command = "runtime_list",
        .help = "Lista todos os runtime data registrados.",
        .hint = NULL,
        .func = &runtime_list,
        .argtable = NULL};

    ESP_ERROR_CHECK(better_console_cmd_register(&runtime_list_cmd));
}
