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
#include "DataManager.hpp"

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
    DataAbstract<bool> *robotPaused;
    DataAbstract<bool> *TunningMode;
    DataAbstract<bool> *HardDeleteMap;
    DataAbstract<bool> *CorrectionTrue;
    DataAbstract<bool> *LineColorBlack;
    DataAbstract<bool> *GD_Optimization;
    DataAbstract<bool> *GD_OptimizationIR;
    
    DataAbstract<bool> *PID_Select; 
    DataAbstract<uint8_t> *TrackStatus; // status (velocidade) do trecho da pista em que o robô se encontra
    DataAbstract<uint8_t> *RealTrackStatus; // status real do trecho da pista em que o robô se encontra
    DataAbstract<bool> *FirstMark; // Verifica se o robô já passou pela primeira marcação lateral
    DataAbstract<bool> *Transition; // Verifica se o robô está numa transição de curva para reta e vice-versa

    DataAbstract<float> *alphaVel; // taxa de aprendizagem do gradiente-descendente para o PID Vel
    DataAbstract<float> *alphaRot; // taxa de aprendizagem do gradiente-descendente para o PID Rot
    DataAbstract<float> *alphaIR; // taxa de aprendizagem do gradiente-descendente para o PIDIR

    DataAbstract<bool> *TuningMapped; // Ativar tuningMode com o mapeamento da pista

    static std::mutex stateMutex;
    
private:
    std::string name;
    DataManager *dataManager;

};

#endif