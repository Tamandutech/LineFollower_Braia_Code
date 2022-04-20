#ifndef SENSORS_SERVICE_H
#define SENSORS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "QTRSensors.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

using namespace cpp_freertos;

//#define LINE_COLOR_BLACK
class SensorsService : public Thread
{
public:
  SensorsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority);

  void Run() override;

private:
  // Componente de gerenciamento dos sensores
  QTRSensors sArray;
  QTRSensors sLat;

  Robot *robot;

  dataSLatMarks *latMarks;
  dataSensor *SLat;
  RobotStatus *status;

  int iloop = 0;

  void calibAllsensors(QTRSensors *sArray, QTRSensors *SLat, Robot *robot);

  void getSensors(QTRSensors *sArray, QTRSensors *SLat, Robot *robot);

  void processSLat(Robot *robot);
};

#endif