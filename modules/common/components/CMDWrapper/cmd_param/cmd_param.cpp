#include <string>

#include "cmd_param.hpp"

#include "DataManager.hpp"
#include "dataEnums.h"

#include "esp_log.h"

#include "better_console.hpp"
#include "argtable3/argtable3.h"

#include "RobotData.h"

static const char *name = "CMD_PARAM";

void register_cmd_param(void)
{
    register_param_set();
    register_param_list();
    register_param_get();
    register_map_get();
    register_map_getRuntime();
    register_map_set();
    register_map_add();
    register_map_SaveRuntime();
    register_map_clear();
    register_map_clearFlash();
    register_map_clearAtIndex();
}

static std::string map_SaveRuntime(int argc, char **argv)
{
    DataMap* MapMarks = Robot::getInstance()->getSLatMarks()->marks;
    MapMarks->saveData();
    return "OK";

}

void register_map_SaveRuntime(void)
{
    const better_console_cmd_t map_SaveRuntime_cmd = {
        .command = "map_SaveRuntime",
        .help = "Salva todos os registros do mapeamento armazenados na Ram.",
        .hint = NULL,
        .func = &map_SaveRuntime,
        .argtable = NULL};

    ESP_ERROR_CHECK(better_console_cmd_register(&map_SaveRuntime_cmd));

}
static struct
{
    struct arg_str *MapString;
    struct arg_end *end;
} map_add_args;

static std::string map_add(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&map_add_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, map_add_args.end, argv[0]);
        return "NOK";
    }
    std::string MapString = map_add_args.MapString->sval[0];
    DataMap* MapMarks = Robot::getInstance()->getSLatMarks()->marks;
    std::stringstream ss(MapString);
    std::string s;
    while (std::getline(ss, s, ';'))
    {
        MapMarks->newData(s);
    }
    return "OK";
}

void register_map_add(void)
{
    map_add_args.MapString = arg_str1(NULL, NULL, "<Mapeamento>", "Registro do mapeamento que será gravado no robô.");
    map_add_args.end = arg_end(2);

    const better_console_cmd_t map_add_cmd = {
        .command = "map_add",
        .help = "Adiciona um registro ao mapeamento.",
        .hint = NULL,
        .func = &map_add,
        .argtable = &map_add_args};

    ESP_ERROR_CHECK(better_console_cmd_register(&map_add_cmd));
}
static struct
{
    struct arg_str *MapString;
    struct arg_end *end;
} map_set_args;

static std::string map_set(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&map_set_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, map_set_args.end, argv[0]);
        return "NOK";
    }
    std::string MapString = map_set_args.MapString->sval[0];
    DataMap* MapMarks = Robot::getInstance()->getSLatMarks()->marks;
    MapMarks->clearAllData();
    std::stringstream ss(MapString);
    std::string s;
    while (std::getline(ss, s, ';'))
    {
        MapMarks->newData(s);
    }
    return "OK";
}

void register_map_set(void)
{
    map_set_args.MapString = arg_str1(NULL, NULL, "<Mapeamento>", "Dados do mapeamento que será gravado no robô.");
    map_set_args.end = arg_end(2);

    const better_console_cmd_t map_set_cmd = {
        .command = "map_set",
        .help = "Altera o valor do mapeamento.",
        .hint = NULL,
        .func = &map_set,
        .argtable = &map_set_args};

    ESP_ERROR_CHECK(better_console_cmd_register(&map_set_cmd));
}
static std::string map_getRuntime(int argc, char **argv)
{
    std::string MapString = "";
    DataMap* MapMarks = Robot::getInstance()->getSLatMarks()->marks;
    for(int i = 0; i < MapMarks->getSize(); i++)
    {
        MapString += MapMarks->getDataString(std::to_string(i)) + '\n';
    }
    return MapString;
}

void register_map_getRuntime(void)
{
    const better_console_cmd_t map_getRuntime_cmd = {
        .command = "map_getRuntime",
        .help = "Retorna todos os registros do mapeamento armazenados na Ram.",
        .hint = NULL,
        .func = &map_getRuntime,
        .argtable = NULL};

    ESP_ERROR_CHECK(better_console_cmd_register(&map_getRuntime_cmd));

}
static std::string map_get(int argc, char **argv)
{
    std::string MapString = "";
    DataMap* MapMarks = Robot::getInstance()->getSLatMarks()->marks;
    MapMarks->loadData();
    for(int i = 0; i < MapMarks->getSize(); i++)
    {
        MapString += MapMarks->getDataString(std::to_string(i)) + '\n';
    }
    return MapString;
}

void register_map_get(void)
{
    const better_console_cmd_t map_get_cmd = {
        .command = "map_get",
        .help = "Retorna todos os registros do mapeamento armazenados na flash.",
        .hint = NULL,
        .func = &map_get,
        .argtable = NULL};

    ESP_ERROR_CHECK(better_console_cmd_register(&map_get_cmd));

}
static std::string map_clear(int argc, char **argv)
{
    DataMap* MapMarks = Robot::getInstance()->getSLatMarks()->marks;
    MapMarks->clearAllData();
    return "OK";
}

void register_map_clear(void)
{
    const better_console_cmd_t map_clear_cmd = {
        .command = "map_clear",
        .help = "limpa todos os registros do mapeamento armazenados na Ram.",
        .hint = NULL,
        .func = &map_clear,
        .argtable = NULL};

    ESP_ERROR_CHECK(better_console_cmd_register(&map_clear_cmd));

}
static std::string map_clearFlash(int argc, char **argv)
{
    DataMap* MapMarks = Robot::getInstance()->getSLatMarks()->marks;
    MapMarks->clearAllData();
    DataStorage::getInstance()->delete_data("sLatMarks.marks");
    return "OK";
}

void register_map_clearFlash(void)
{
    const better_console_cmd_t map_clearFlash_cmd = {
        .command = "map_clearFlash",
        .help = "limpa todos os registros do mapeamento armazenados na flash e na Ram.",
        .hint = NULL,
        .func = &map_clearFlash,
        .argtable = NULL};

    ESP_ERROR_CHECK(better_console_cmd_register(&map_clearFlash_cmd));

}
static struct
{
    struct arg_str *pos;
    struct arg_end *end;
} map_clearAtIndex_args;

static std::string map_clearAtIndex(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&map_clearAtIndex_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, map_clearAtIndex_args.end, argv[0]);
        return "NOK";
    }
    DataMap* MapMarks = Robot::getInstance()->getSLatMarks()->marks;
    MapMarks->clearData(std::stoi(map_clearAtIndex_args.pos->sval[0]));
    return "OK";
}

void register_map_clearAtIndex(void)
{
    map_clearAtIndex_args.pos = arg_str1(NULL, NULL, "<pos>", "index do registro do mapeamento que será deletado da ram.");
    map_clearAtIndex_args.end = arg_end(2);

    const better_console_cmd_t map_clearAtIndex_cmd = {
        .command = "map_clearAtIndex",
        .help = "deleta um registro do mapeamento na ram em uma determinada posição.",
        .hint = NULL,
        .func = &map_clearAtIndex,
        .argtable = &map_clearAtIndex_args};

    ESP_ERROR_CHECK(better_console_cmd_register(&map_clearAtIndex_cmd));
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
    CarState state = (CarState)Robot::getInstance()->getStatus()->robotState->getData();
    
    if(state == CAR_STOPPED) DataManager::getInstance()->setParam(param_set_args.name->sval[0], param_set_args.value->sval[0]);
    else 
    {
        DataManager::getInstance()->registerParamDataChanged(param_set_args.name->sval[0]);
        DataManager::getInstance()->setParam(param_set_args.name->sval[0], param_set_args.value->sval[0],false);
    }
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

    ESP_ERROR_CHECK(better_console_cmd_register(&param_set_cmd));
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

    ESP_ERROR_CHECK(better_console_cmd_register(&param_get_cmd));
}

static std::string param_list(int argc, char **argv)
{
    std::string ret = DataManager::getInstance()->listRegistredParamData();
    ESP_LOGD(name, "Tamanho da lista: %d", ret.length());
    return ret;
}

void register_param_list(void)
{
    const better_console_cmd_t param_list_cmd = {
        .command = "param_list",
        .help = "Lista todos os parâmetros registrado.",
        .hint = NULL,
        .func = &param_list,
        .argtable = NULL};

    ESP_ERROR_CHECK(better_console_cmd_register(&param_list_cmd));
}