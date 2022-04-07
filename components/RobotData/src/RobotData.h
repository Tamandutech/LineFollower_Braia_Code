#ifndef ROBOT_DATA_H
#define ROBOT_DATA_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <queue>

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include "dataSLatMarks.h"
#include "dataSpeed.h"
#include "dataPID.h"
#include "dataSensor.h"
#include "RobotStatus.h"

#include "esp_log.h"

#include "DataStorage.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

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
    struct PacketData getPacketSend();
    int addPacketSend(struct PacketData packet);
    bool PacketSendavailable();

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
    std::queue<struct PacketData> PacketstoSend; // Pacotes para envio pelo espnow
    SemaphoreHandle_t xSemaphorepacketstosend;
};

#endif