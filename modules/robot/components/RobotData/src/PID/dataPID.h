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

#include "esp_log.h"

class dataPID
{
public:
    dataPID(std::string name = "dataPID");

    DataAbstract<float> *input;
    DataAbstract<float> *output;
    DataAbstract<float> *P_output;
    DataAbstract<float> *I_output;
    DataAbstract<float> *D_output;
    DataAbstract<float> *setpoint;
    DataAbstract<float> *erro;
    DataAbstract<float> *erroquad;

    // Parâmetros do PID
    DataAbstract<double> *Kp_acceleration;
    DataAbstract<double> *Kp_desacceleration;
    DataAbstract<double> *Kp_default;
    DataAbstract<double> *Ki_default;
    DataAbstract<double> *Kd_default;

    // Parâmetros do PID para diferentes trechos da pista
    DataAbstract<double> *Kp_tunning; 
    DataAbstract<double> *Ki_tunning;
    DataAbstract<double> *Kd_tunning; 

    DataAbstract<double> *Kp_line; 
    DataAbstract<double> *Kd_line; 

    DataAbstract<double> *Kp_curve; 
    DataAbstract<double> *Kd_curve;

    DataAbstract<double> *Kp_LongCurve; 
    DataAbstract<double> *Kd_LongCurve; 

    DataAbstract<double> *Kp_ZigZag;
    DataAbstract<double> *Kd_ZigZag;

private:
    std::string name;
    const char *tag = "RobotData";

   
    DataManager *dataManager;
};

#endif