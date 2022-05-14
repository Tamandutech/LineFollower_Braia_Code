#ifndef SENSORS_SERVICE_H

#define SENSORS_SERVICE_H

#include "thread.hpp"
#include "RobotData.h"
#include "LEDsService.hpp"

#include "QTRSensors.h"

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

#define LINE_COLOR_BLACK

using namespace cpp_freertos;

class SensorsService : public Thread
{
public:
  static SensorsService *getInstance(std::string name = "SensorsService", uint32_t stackDepth = 10000, UBaseType_t priority = 9)
    {
        SensorsService *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
            std::lock_guard<std::mutex> myLock(instanceMutex);
            sin = instance.load(std::memory_order_relaxed);
            if (!sin)
            {
                sin = new SensorsService(name, stackDepth, priority);
                instance.store(sin, std::memory_order_release);
            }
        }

        return sin;
    };

  void Run() override;
  void calibAllsensors();


private:

  static std::atomic<SensorsService *> instance;
  static std::mutex instanceMutex;

  // Componente de gerenciamento dos sensores
  QTRSensors sArray;
  QTRSensors sLat;

  Robot *robot;

  dataSLatMarks *latMarks;
  dataSensor *SLat;
  RobotStatus *status;

  int iloop = 0;

  void getSensors(QTRSensors *sArray, QTRSensors *SLat, Robot *robot);

  void processSLat(Robot *robot);

  SensorsService(std::string name, uint32_t stackDepth, UBaseType_t priority);
};

#endif