#ifndef ROBOT_DATA_H
#define ROBOT_DATA_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <queue>
#include <atomic>
#include <mutex>

#include "dataSLatMarks.h"
#include "dataSpeed.h"
#include "dataPID.h"
#include "dataSensor.h"
#include "RobotStatus.h"

#include "DataStorage.hpp"
#include "DataManager.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
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
    dataPID *getPIDVel();
    dataPID *getPIDRot();
    RobotStatus *getStatus();
    dataSLatMarks *getSLatMarks();

private:
    std::string name;

    static std::atomic<Robot *> instance;
    static std::mutex instanceMutex;

    int Updateparams(struct CarParameters params);

    dataSpeed *speed;
    dataPID *PIDVel;
    dataPID *PIDRot;
    dataSensor *sLat;
    dataSLatMarks *sLatMarks;
    dataSensor *sArray;
    RobotStatus *Status;

    DataStorage *storage;
    DataManager *dataManager;

    Robot(std::string name);
};

#endif