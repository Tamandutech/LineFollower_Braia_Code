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
  uint16_t getArraySensors();


private:

  // Componente de gerenciamento dos sensores
  QTRSensors sArray;
  QTRSensors sLat;

  Robot *robot;

  dataMapping *MappingData;
  dataSensor *sLatData;
  RobotStatus *status;
  
  int sumSensEsq = 0;
  int sumSensDir = 0;
  int MarksToMean = 0;

  int latloop = 0;
  int sloop = 0;

  int nLatReads = 0;

  void getLatSensors();

  void processSLat();

};

#endif