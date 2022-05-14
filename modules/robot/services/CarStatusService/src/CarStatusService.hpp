#ifndef CAR_STATUS_SERVICE_H
#define CAR_STATUS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"
#include "dataEnums.h"
#include "driver/gpio.h"

#include "MappingService.hpp"
#include "LEDsService.hpp"
#include "RobotData.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

#define ManualMap

class CarStatusService : public Thread
{
public:
    static CarStatusService *getInstance(std::string name = "CarStatusService", uint32_t stackDepth = 10000, UBaseType_t priority = 19)
    {
        CarStatusService *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
            std::lock_guard<std::mutex> myLock(instanceMutex);
            sin = instance.load(std::memory_order_relaxed);
            if (!sin)
            {
                sin = new CarStatusService(name, stackDepth, priority);
                instance.store(sin, std::memory_order_release);
            }
        }

        return sin;
    };

    void Run() override;

private:
    static std::atomic<CarStatusService *> instance;
    static std::mutex instanceMutex;

    CarStatusService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    Robot *robot;
    RobotStatus *status;
    dataSpeed *speed;
    dataSLatMarks *latMarks;
    dataPID *PidTrans;

    CarState actualCarState;

    MappingService *mappingService;

    int numMarks = 0; // Número total de marcações laterais na pista

    bool stateChanged; // verifica se o carrinho mudou seu estado quanto ao mapeamento

    uint8_t lastState; // armazena último estado do mapeamento
    bool lastMappingState;

    int32_t mediaEncActual = 0;
    int32_t mediaEncFinal = 0;


    static QueueHandle_t gpio_evt_queue;
    static void IRAM_ATTR gpio_isr_handler(void *arg);
};

#endif