#ifndef ROBOT_DATA_H
#define ROBOT_DATA_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <queue>

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
    Robot(std::string name = "NONAME");
    dataSpeed *getSpeed();
    dataSensor *getsLat();
    dataSensor *getsArray();
    dataPID *getPIDVel();
    dataPID *getPIDRot();
    RobotStatus *getStatus();
    dataSLatMarks *getSLatMarks();

private:
    std::string name;
    const char *tag = "RobotData";

    int Updateparams(struct CarParameters params);

    dataSpeed *speed;
    dataPID *PIDVel;
    dataPID *PIDRot;
    dataSensor *sLat;
    dataSLatMarks *sLatMarks;
    dataSensor *sArray;
    RobotStatus *Status;
    struct CarParameters Carparam; // Parâmetros do robô

    DataStorage *storage;
    DataManager *dataManager;
};

#endif