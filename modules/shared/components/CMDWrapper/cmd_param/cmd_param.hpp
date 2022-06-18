#pragma once

#include <string>

void register_cmd_param(void);

static std::string param_set(int argc, char **argv);
static std::string map_set(int argc, char **argv);
static std::string map_add(int argc, char **argv);
static std::string map_clearAtIndex(int argc, char **argv);

void register_param_set(void);
void register_param_list(void);
void register_param_get(void);
void register_map_get(void);
void register_map_getRuntime(void);
void register_map_set(void);
void register_map_add(void);
void register_map_SaveRuntime(void);
void register_map_clear(void);
void register_map_clearFlash(void);
void register_map_clearAtIndex(void);