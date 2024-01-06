#ifndef DATA_SENSOR_H 
#define DATA_SENSOR_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "dataEnums.h"
#include "DataStorage.hpp"

#include "esp_log.h"

class dataSensor
{
public:
    dataSensor(uint16_t sensorNumber, std::string name = "dataSensor");

    int setLine(uint16_t value);
    uint16_t getLine();

    int setChannel(uint16_t sensorPosition, uint16_t value, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg);
    int setChannel(uint16_t sensorPosition, uint16_t value);
    uint16_t getChannel(uint16_t sensorPosition, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg);
    uint16_t getChannel(uint16_t sensorPosition);

    int setChannels(std::vector<uint16_t> values, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg);
    int setChannels(std::vector<uint16_t> values);
    std::vector<uint16_t> getChannels(std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg);
    std::vector<uint16_t> getChannels();

private:
    std::string name;
    const char *tag = "RobotData";

    // Valores
    SemaphoreHandle_t xSemaphorechannel;
    std::vector<uint16_t> channel;

    SemaphoreHandle_t xSemaphoreline;
    uint16_t line;

    // Parâmetros
    uint16_t sensorNumber = 0;

    esp_err_t  checksIfSensorExisty(uint16_t sensorPosition);
    bool isSensorAvailableForReading(SemaphoreHandle_t *xSemaphoreOfArg);

};

#endif