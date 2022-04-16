#ifndef DATA_PID_H
#define DATA_PID_H

#define DEFAULT_SETPOINT 3500

#define DEFAULT_KP_LINE 3500
#define DEFAULT_KP_CURVE 3500

#define DEFAULT_KI_LINE 3500
#define DEFAULT_KI_CURVE 3500

#define DEFAULT_KD_LINE 3500
#define DEFAULT_KD_CURVE 3500

#include <stdint.h>
#include <stddef.h>
#include <string>

#include "dataEnums.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "DataAbstract.hpp"
#include "DataStorage.hpp"
#include "DataManager.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

class dataPID
{
public:
    dataPID(std::string name = "dataPID");

    DataAbstract<int16_t> *input;
    DataAbstract<float> *output;
    DataAbstract<int16_t> *setpoint; // salvar

    DataAbstract<float> *Kp(CarState state);
    DataAbstract<float> *Ki(CarState state);
    DataAbstract<float> *Kd(CarState state);

private:
    std::string name;
    const char *tag = "RobotData";

    DataAbstract<float> *Kp_line; // salvar
    DataAbstract<float> *Ki_line; // salvar
    DataAbstract<float> *Kd_line; // salvar

    DataAbstract<float> *Kp_curve; // salvar
    DataAbstract<float> *Ki_curve; // salvar
    DataAbstract<float> *Kd_curve; // salvar

    DataManager *dataManager;
};

#endif