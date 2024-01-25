#ifndef MAPPING_SERVICE_H
#define MAPPING_SERVICE_H

#include <atomic>
#include <mutex>
#include <math.h>
#include <limits.h>

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"
#include "TrackSegment.hpp"
#include "LEDsService.hpp"
#include "SpeedService.hpp"

#include "driver/gpio.h"

using namespace cpp_freertos;

#include "esp_log.h"

class MappingService : public Thread, public Singleton<MappingService>
{
public:

    MappingService(std::string name, uint32_t stackDepth, UBaseType_t priority);
    
    void Run() override;

    esp_err_t startNewMapping();
    esp_err_t stopNewMapping();

    esp_err_t loadMapping();
    esp_err_t saveMapping();

    esp_err_t createNewMark();

private:
    std::string name;

    Robot *robot;
    dataSpeed *speed;
    dataMapping *MappingData;
    RobotStatus *status;

    MapData currentMark;

    uint16_t rightMarksToStop;

    // variáveis auxiliares para cálculos0;
    int32_t EncLeft = 0, EncRight = 0;
    int32_t lastEncLeft = 0, lastEncRight = 0, lastmarkPosition = 0;

    LedColor color;
    led_position_t led;

};

#endif