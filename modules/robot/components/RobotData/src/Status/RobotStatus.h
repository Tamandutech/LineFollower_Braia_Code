#ifndef ROBOT_STATUS_H
#define ROBOT_STATUS_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>

#include "dataEnums.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "DataAbstract.hpp"
#include "DataStorage.hpp"

#include "esp_log.h"

class RobotStatus
{
public:
    RobotStatus(CarState initialState, std::string name);

    /**
     * @brief Armazena o estado geral do robô.
     * @returns Valor do tipo RobotState.
     */
    DataAbstract<uint8_t> *robotState;

    /**
     * @brief Atributo que indica se o robô está mapeando a pista
     * @retval TRUE Se o robô está mapeando a pista
     * @retval FALSE Se o robô não está mapeando a pista
     */
    DataAbstract<bool> *robotIsMapping;
     /**
     * @brief Atributo que indica se o robô está lendo o mapeamento para identificar as curvas e retas
     * @retval TRUE Se o robô está lendo o mapeamento
     * @retval FALSE Se o robô não está lendo o mapeamento
     */
    DataAbstract<bool> *encreading;

    static std::mutex stateMutex;
    
private:
    std::string name;

};

#endif