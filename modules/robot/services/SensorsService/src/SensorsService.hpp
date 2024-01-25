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
  void calibrateAllSensors();
  uint16_t UpdateFrontSensors();


private:

  // Componente de gerenciamento dos sensores
  QTRSensors frontSensors;
  QTRSensors sideSensors;

  Robot *robot;

  dataMapping *MappingData;
  dataSensor *sideSensorsData;
  RobotStatus *status;
  
  int sumReadLeftSensor = 0;
  int sumReadRightSensor = 0;
  int targetNumberSensorReadsToMean = 0;

  int latloop = 0;
  int sloop = 0;

  int sensorsReadNumber = 0;

  void calibrateSensors(QTRSensors *sensorIR, LedColor color);
  void UpdateSideSensors();
  void processSideSensors();

  void processSensorData(int meanSensEsq, int meanSensDir);
  void processSensorsReadingMark(int meanSensEsq, int meanSensDir);
  void processBothSensorsReadingMark();
  void processLefSensorReadingMark();
  void processRightSensorReadingMark();
  void processSensorsReadingNothing();
  void resetSensorData();
  bool isWhite(int sensorValue);
  bool isBlack(int sensorValue);
};

#endif