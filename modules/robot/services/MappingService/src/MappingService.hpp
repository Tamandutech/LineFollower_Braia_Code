#ifndef MAPPING_SERVICE_H
#define MAPPING_SERVICE_H

#include <atomic>
#include <mutex>
#include <math.h>
#include <limits.h>

#include "thread.hpp"
#include "RobotData.h"
#include "LEDsService.hpp"

#include "driver/gpio.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#define ManualMap

class MappingService : public Thread<MappingService>
{
public:
    static MappingService *getInstance(std::string name = "MappingService", uint32_t stackDepth = 10000, UBaseType_t priority = 9)
    {
        MappingService *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
            std::lock_guard<std::mutex> myLock(instanceMutex);
            sin = instance.load(std::memory_order_relaxed);
            if (!sin)
            {
                sin = new MappingService(name, stackDepth, priority);
                instance.store(sin, std::memory_order_release);
            }
        }

        return sin;
    };

    void Run() override;

    esp_err_t startNewMapping(uint8_t leftMarksToStop = CHAR_MAX, int32_t mediaPulsesToStop = LONG_MAX, uint32_t timeToStop = (portMAX_DELAY / portTICK_PERIOD_MS));
    esp_err_t stopNewMapping();

    esp_err_t loadMapping();
    esp_err_t saveMapping();

    esp_err_t createNewMark();

private:
    std::string name;

    static std::atomic<MappingService *> instance;
    static std::mutex instanceMutex;

    Robot *robot;
    dataSpeed *speedMapping;
    dataSensor *sLat;
    dataSLatMarks *latMarks;
    RobotStatus *status;

    struct MapData tempActualMark;
    struct MapData tempPreviousMark;

    // atributos de filtro
    uint8_t leftMarksToStop;
    int32_t mediaPulsesToStop;
    TickType_t ticksToStop;

    // atributos para offsets iniciais
    int32_t initialRightPulses = 0;
    int32_t initialLeftPulses = 0;
    int32_t initialMediaPulses = 0;
    TickType_t initialTicks = 0;

    // variáveis de calculos temporárias
    uint32_t tempDeltaPulses = 0;
    uint32_t tempMilimiterInPulses = 0;

    led_command_t command;

    MappingService(std::string name, uint32_t stackDepth, UBaseType_t priority);
};

#endif