#ifndef SENSORS_SERVICE_H
#define SENSORS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"

#include "io.h"

#include "QTRSensors.h"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR

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

  void calibAllsensors(QTRSensors *sArray, QTRSensors *SLat, Robot *robot);

  void getSensors(QTRSensors *sArray, QTRSensors *SLat, Robot *robot);

  void processSLat(Robot *robot);
};

#endif