#include <string>

#include "cmd_param.hpp"

#include "ESPNOWHandler.h"
#include "DataManager.hpp"
#include "dataEnums.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#include "better_console.hpp"
#include "argtable3/argtable3.h"

static const char *name = "CMD_PARAM";

void register_cmd_param(void)
{
    register_param_set();
    register_param_list();
    register_param_get();
}

static struct
{
    struct arg_str *name;
    struct arg_str *value;
    struct arg_end *end;
} param_set_args;

static std::string param_set(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&param_set_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, param_set_args.end, argv[0]);
        return "NOK";
    }

    ESP_LOGD(name, "Definido parâmetro local %s com valor %s", param_set_args.name->sval[0], param_set_args.value->sval[0]);
    DataManager::getInstance()->setParam(param_set_args.name->sval[0], param_set_args.value->sval[0]);

    return "OK";
}

void register_param_set(void)
{
    param_set_args.name = arg_str1(NULL, NULL, "<parâmetro>", "Nome do parâmetro a ser alterado no robô.");
    param_set_args.value = arg_str1(NULL, NULL, "<valor>", "Valor a ser atribuido ao parametro.");
    param_set_args.end = arg_end(2);

    const better_console_cmd_t param_set_cmd = {
        .command = "param_set",
        .help = "Altera o valor de um parametro.",
        .hint = NULL,
        .func = &param_set,
        .argtable = &param_set_args};

    better_console_cmd_register(&param_set_cmd);
}

static struct
{
    struct arg_str *name;
    struct arg_str *ctrl;
    struct arg_end *end;
} param_get_args;

static std::string param_get(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&param_get_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, param_get_args.end, argv[0]);
        return "NOK";
    }

    ESP_LOGD(name, "Buscando parâmetro local %s com valor %s", param_get_args.name->sval[0], param_get_args.ctrl->sval[0]);

    return DataManager::getInstance()->getParam(param_get_args.name->sval[0], param_get_args.ctrl->sval[0]);
}

void register_param_get(void)
{
    param_get_args.name = arg_str1(NULL, NULL, "<parâmetro>", "Nome do parâmetro a ser alterado no robô.");
    param_get_args.ctrl = arg_str0("c", "ctrl", "<ctrl>", "String de controle para buscar o parâmetro.");
    param_get_args.end = arg_end(2);

    const better_console_cmd_t param_get_cmd = {
        .command = "param_get",
        .help = "Busca o valor de um parametro.",
        .hint = NULL,
        .func = &param_get,
        .argtable = &param_get_args};

    better_console_cmd_register(&param_get_cmd);
}

static std::string param_list(int argc, char **argv)
{
    DataManager::getInstance()->listRegistredParamData();

    return 0;
}

void register_param_list(void)
{
    const better_console_cmd_t param_list_cmd = {
        .command = "param_list",
        .help = "Lista todos os parâmetros registrado.",
        .hint = NULL,
        .func = &param_list,
        .argtable = NULL};

    better_console_cmd_register(&param_list_cmd);
}