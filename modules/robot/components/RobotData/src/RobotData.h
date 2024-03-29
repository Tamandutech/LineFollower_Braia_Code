#ifndef ROBOT_DATA_H
#define ROBOT_DATA_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <queue>
#include <atomic>
#include <mutex>

#include "esp_adc/adc_oneshot.h"

#include "dataMapping.h"
#include "dataSpeed.h"
#include "dataPID.h"
#include "dataSensor.h"
#include "RobotStatus.h"

#include "DataStorage.hpp"
#include "DataManager.hpp"

#include "esp_log.h"

class Robot
{
public:
    static Robot *getInstance(std::string name = "RobotData")
    {
        Robot *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
            std::lock_guard<std::mutex> myLock(instanceMutex);
            sin = instance.load(std::memory_order_relaxed);
            if (!sin)
            {
                sin = new Robot(name);
                instance.store(sin, std::memory_order_release);
            }
        }

        return sin;
    };

    dataSpeed *getSpeed();
    dataSensor *getsLat();
    dataSensor *getsArray();
    dataPID *getPID();
    RobotStatus *getStatus();
    dataMapping *getMappingData();
    adc_oneshot_unit_handle_t getADC_handle();
    std::string GetName();

private:
    std::string name;

    static std::atomic<Robot *> instance;
    static std::mutex instanceMutex;

    adc_oneshot_unit_handle_t _adcHandle;

    dataSpeed *speed;
    dataPID *PID; 
    dataSensor *sLat;
    dataMapping *MappingData;
    dataSensor *sArray;
    RobotStatus *Status;
    DataStorage *storage;
    DataManager *dataManager;

    Robot(std::string name);
};

#endif