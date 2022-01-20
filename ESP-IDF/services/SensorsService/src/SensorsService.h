#ifndef SENSORS_SERVICE_H
#define SENSORS_SERVICE_H

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "Service.h"

#include "io.h"

#include "QTRSensors.h"

class SensorsService : public Service
{
public:
  SensorsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Service(name, robot, stackDepth, priority){};
  //~SensorsService(){};

  void Setup() override;
  void Main() override;

private:
  // Componente de gerenciamento dos sensores
  QTRSensors sArray;
  QTRSensors sLat;

  void calibAllsensors(QTRSensors *sArray, QTRSensors *SLat, Robot *robot);

  void getSensors(QTRSensors *sArray, QTRSensors *SLat, Robot *robot);

  void processSLat(Robot *robot);
};

#endif