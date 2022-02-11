#ifndef CAR_STATUS_SERVICE_H
#define CAR_STATUS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "driver/gpio.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

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

    int Marks = 0;

    bool mapChanged; // verifica se o carrinho mudou seu estado quanto ao mapeamento
    bool lastmapstate; // armazena último estado do mapeamento

    // Matriz com dados de media encoders,linha do carrinho
    int32_t FinalMark = 0;                        // Media dos encoders da marcação final
    int32_t PlusPulses = 0;                       // Pulsos a mais para a parada
};

#endif