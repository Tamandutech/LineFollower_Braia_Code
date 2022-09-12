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
    dataSensor(uint16_t qtdChannels, std::string name = "dataSensor");

    int setLine(uint16_t value);
    uint16_t getLine();

    int setChannel(uint16_t channelNumber, uint16_t value, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg);
    int setChannel(uint16_t channelNumber, uint16_t value);
    uint16_t getChannel(uint16_t channelNumber, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg);
    uint16_t getChannel(uint16_t channelNumber);

    int setChannels(std::vector<uint16_t> values, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg);
    int setChannels(std::vector<uint16_t> values);
    std::vector<uint16_t> getChannels(std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg);
    std::vector<uint16_t> getChannels();

    int setChannelMin(uint16_t channelNumber, uint16_t value);
    uint16_t getChannelMin(uint16_t channelNumber);

    int setChannelMax(uint16_t channelNumber, uint16_t value);
    uint16_t getChannelMax(uint16_t channelNumber);

    int setChannelsMins(std::vector<uint16_t> values);
    std::vector<uint16_t> getChannelsMins();

    int setChannelsMaxes(std::vector<uint16_t> values);
    std::vector<uint16_t> getChannelsMaxes();

private:
    std::string name;
    const char *tag = "RobotData";

    // Valores
    SemaphoreHandle_t xSemaphorechannel;
    std::vector<uint16_t> channel;

    SemaphoreHandle_t xSemaphoreline;
    uint16_t line;

    // Parâmetros
    uint16_t qtdChannels = 0;

    SemaphoreHandle_t xSemaphoremaxChannel;
    std::vector<uint16_t> maxChannel;

    SemaphoreHandle_t xSemaphoreminChannel;
    std::vector<uint16_t> minChannel;
};

#endif