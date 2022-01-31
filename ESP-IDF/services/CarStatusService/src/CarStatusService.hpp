#ifndef CAR_STATUS_SERVICE_H
#define CAR_STATUS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

using namespace cpp_freertos;

#define Marks 40 // marcas laterais esquerda na pista

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

class CarStatusService : public Thread
{
public:
    CarStatusService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;

private:
    Robot *robot;
    RobotStatus *status;
    dataSpeed *speed;

    // Matriz com dados de media encoders,linha do carrinho
    int32_t Manualmap[2][40] = {{0, 0, 0, 0, 0},  // media
                                {0, 0, 0, 0, 0}}; // linha
    int32_t FinalMark = 0;                        // Media dos encoders da marcação final
    int32_t PlusPulses = 0;                       // Pulsos a mais para a parada
};

#endif