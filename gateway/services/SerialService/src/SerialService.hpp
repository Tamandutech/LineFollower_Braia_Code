#ifndef SERIAL_SERVICE_HPP
#define SERIAL_SERVICE_HPP

#include "thread.hpp"

#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "cmd_decl.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include "DataStorage.hpp"

static const char *TAG = "example";
#define PROMPT_STR "robotGateway"

#define HISTORY_PATH "/data/history.txt"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#define ManualMap

class SerialService : public Thread
{
public:
    SerialService(const char *name, uint32_t stackDepth, UBaseType_t priority);
    void Run() override;

private:
    DataStorage *dataStorage;

    static void initialize_console(void);
};

#endif