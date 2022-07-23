#ifndef SENSORS_SERVICE_H

#define SENSORS_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"
#include "LEDsService.hpp"

#include "QTRSensors.h"

#include "esp_log.h"

//#define LINE_COLOR_BLACK

using namespace cpp_freertos;

class SensorsService : public Thread, public Singleton<SensorsService>
{
public:

  SensorsService(std::string name, uint32_t stackDepth, UBaseType_t priority);

  void Run() override;
  void calibAllsensors();


private:

  // Componente de gerenciamento dos sensores
  QTRSensors sArray;
  QTRSensors sLat;

  Robot *robot;

  dataSLatMarks *latMarks;
  dataSensor *SLat;
  RobotStatus *status;

  led_command_t command;
  
  int iloop = 0;

  void getSensors(QTRSensors *sArray, QTRSensors *SLat, Robot *robot);

  void processSLat(Robot *robot);

};

#endif