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
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "DataAbstract.hpp"
#include "DataStorage.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

class dataPID
{
public:
    dataPID(std::string name = "dataPID");

    DataAbstract<int16_t> *input;
    DataAbstract<float> *output;
    DataAbstract<int16_t> *setpoint;

    DataAbstract<float> *Kp(CarState state);
    DataAbstract<float> *Ki(CarState state);
    DataAbstract<float> *Kd(CarState state);

private:
    std::string name;
    const char *tag = "RobotData";
  
    DataAbstract<float> *Kp_line;
    DataAbstract<float> *Ki_line;
    DataAbstract<float> *Kd_line;

    DataAbstract<float> *Kp_curve;
    DataAbstract<float> *Ki_curve;
    DataAbstract<float> *Kd_curve;
};

#endif