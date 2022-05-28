#pragma once

#include <string>

void register_cmd_param(void);

static std::string param_set(int argc, char **argv);

void register_param_set(void);
void register_param_list(void);
void register_param_get(void);