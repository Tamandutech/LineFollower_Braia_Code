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

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

class dataPID
{
public:
    dataPID(std::string name = "dataPID");

    int setInput(int16_t input);
    int16_t getInput();
    
    int setOutput(float output);
    float getOutput();

    int setSetpoint(int16_t Setpoint);
    int16_t getSetpoint();

    int setKp(float Kp, CarState state);
    float getKp(CarState state);

    int setKi(float Ki, CarState state);
    float getKi(CarState state);

    int setKd(float Kd, CarState state);
    float getKd(CarState state);

private:
    std::string name;
    const char *tag = "RobotData";

    // Entrada e Saída da função
    SemaphoreHandle_t xSemaphoreInput;
    int16_t input = 0;
    SemaphoreHandle_t xSemaphoreOutput;
    float output = 0;

    // Parâmetros
    SemaphoreHandle_t xSemaphoreSetpoint;
    int16_t Setpoint;

    SemaphoreHandle_t xSemaphoreKp_line;
    float Kp_line;
    SemaphoreHandle_t xSemaphoreKi_line;
    float Ki_line;
    SemaphoreHandle_t xSemaphoreKd_line;
    float Kd_line;
    
    SemaphoreHandle_t xSemaphoreKp_curve;
    float Kp_curve;
    SemaphoreHandle_t xSemaphoreKi_curve;
    float Ki_curve;
    SemaphoreHandle_t xSemaphoreKd_curve;
    float Kd_curve;
};

#endif