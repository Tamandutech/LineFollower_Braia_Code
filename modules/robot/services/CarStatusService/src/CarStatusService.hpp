#ifndef CAR_STATUS_SERVICE_H
#define CAR_STATUS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"
#include "dataEnums.h"
#include "driver/gpio.h"

#include "MappingService.hpp"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

#define ManualMap

class CarStatusService : public Thread
{
public:
    CarStatusService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);
    void Run() override;

private:
    Robot *robot;
    RobotStatus *status;
    dataSpeed *speed;
    dataSLatMarks *latMarks;
    dataPID *PidTrans;

    CarState actualCarState;

    MappingService *mappingService;

    int Marks = 0;

    bool stateChanged; // verifica se o carrinho mudou seu estado quanto ao mapeamento

    uint8_t lastState; // armazena último estado do mapeamento
    bool lastMappingState;

    int32_t mediaEncActual = 0;
    int32_t mediaEncFinal = 0;

    static QueueHandle_t gpio_evt_queue;
    static void IRAM_ATTR gpio_isr_handler(void *arg);
};

#endif