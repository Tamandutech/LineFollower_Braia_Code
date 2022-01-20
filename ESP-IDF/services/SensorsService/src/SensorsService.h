#ifndef SENSORS_SERVICE_H
#define SENSORS_SERVICE_H

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "thread.hpp"
#include "RobotData.h"

#include "io.h"

#include "QTRSensors.h"

using namespace cpp_freertos;

class SensorsService : public Thread
{
public:
  SensorsService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
  {
    this->robot = robot;

    // Definindo GPIOs e configs para sensor Array
    sArray.setTypeMCP3008();
    sArray.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, 8, (gpio_num_t)ADC_DOUT, (gpio_num_t)ADC_DIN, (gpio_num_t)ADC_CLK, (gpio_num_t)ADC_CS, 1350000, VSPI_HOST);
    sArray.setSamplesPerSensor(5);

    // Definindo GPIOs e configs para sensor Lateral
    gpio_pad_select_gpio(39);
    gpio_set_direction(GPIO_NUM_17, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(05);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);

    sLat.setTypeAnalogESP();
    sLat.setSensorPins((const adc1_channel_t[]){SL1, SL2}, 2);
    sLat.setSamplesPerSensor(5);

    //Calibração dos dos sensores laterais e array
    for (uint16_t i = 0; i < 20; i++)
    {
      ESP_LOGD(GetName().c_str(), "(%p) | sArray: (%p) | sLat: (%p)", this, &sArray, &sLat);
      sArray.calibrate();
      sLat.calibrate();
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    //leitura e armazenamento dos valores máximos e mínimos dos sensores obtidos na calibração
    std::vector<uint16_t> sArrayMaxes(sArray.calibrationOn.maximum, sArray.calibrationOn.maximum + sArray.getSensorCount());
    std::vector<uint16_t> sArrayMins(sArray.calibrationOn.minimum, sArray.calibrationOn.minimum + sArray.getSensorCount());
    std::vector<uint16_t> SLatMaxes(sLat.calibrationOn.maximum, sLat.calibrationOn.maximum + sLat.getSensorCount());
    std::vector<uint16_t> SLatMins(sLat.calibrationOn.minimum, sLat.calibrationOn.minimum + sLat.getSensorCount());

    //armazenamento dos valores máximos e mínimos dos sensores no objeto robot
    robot->getsArray()->setChannelsMaxes(sArrayMaxes);
    robot->getsArray()->setChannelsMins(sArrayMins);
    robot->getsLat()->setChannelsMaxes(SLatMaxes);
    robot->getsLat()->setChannelsMins(SLatMins);

    // vTaskResume(xTaskMotors);
    // vTaskResume(xTaskPID);
    // if (taskStatus)
    //     vTaskResume(xTaskCarStatus);
    // vTaskResume(xTaskSpeed);
    // if (robot->getStatus()->getMapping())
    //     vTaskResume(xTaskMapping);
  };

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