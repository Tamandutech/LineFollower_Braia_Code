#ifndef ROBOT_DATA_H
#define ROBOT_DATA_H

#include <stdint.h>
#include <stddef.h>
#include <string>

#include "dataSLatMarks.h"
#include "dataSpeed.h"
#include "dataPID.h"
#include "dataSensor.h"
#include "RobotStatus.h"

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

    dataSpeed *speed;
    dataPID *PIDVel;
    dataPID *PIDRot;
    dataSensor *sLat;
    dataSLatMarks *sLatMarks;
    dataSensor *sArray;
    RobotStatus *Status;
};

#endif