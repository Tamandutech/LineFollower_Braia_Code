#ifndef SERIAL_SERVICE_HPP
#define SERIAL_SERVICE_HPP

#include "thread.hpp"
#include "singleton.hpp"

#include "better_console.hpp"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include "DataStorage.hpp"

#define PROMPT_STR "robotGateway"

#define HISTORY_PATH "/data/history.txt"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

#define ManualMap

class SerialService : public Thread, public Singleton<SerialService>
{
public:
    SerialService(std::string name, uint32_t stackDepth, UBaseType_t priority);
    void Run() override;

private:
    DataStorage *dataStorage;

    static void initialize_console(void);
};

#endif