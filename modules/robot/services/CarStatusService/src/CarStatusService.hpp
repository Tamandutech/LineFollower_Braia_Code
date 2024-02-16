#ifndef CAR_STATUS_SERVICE_H
#define CAR_STATUS_SERVICE_H

#include "thread.hpp"
#include "singleton.hpp"
#include "RobotData.h"
#include "dataEnums.h"
#include "TrackSegment.hpp"
#include "driver/gpio.h"

#include "MappingService.hpp"
#include "LEDsService.hpp"
#include "SpeedService.hpp"


using namespace cpp_freertos;
#include "esp_log.h"

class CarStatusService : public Thread, public Singleton<CarStatusService>
{
public:
    
    CarStatusService(std::string name, uint32_t stackDepth, UBaseType_t priority);

    void Run() override;
    static SemaphoreHandle_t SemaphoreStartRobot;


private:

    Robot *robot;
    RobotStatus *status;
    dataSpeed *speed;
    dataMapping *MappingData;
    dataPID *PidTrans;

    CarState initialRobotState, currentRobotState, previousRobotState;

    MappingService *mappingService;

    int TotalMarksNumber;
    
    int printInterval; // variável para o controle do intervalo entre os prints
    
    bool inTransition, previouslyInTransition;

    TrackSegment transitionTrackSegment; // trecho em que o robô estava ou estará
    TrackSegment previousTrack; 

    MapData finalMark;
    int32_t robotPosition;
    int16_t previousMarkoffset, currentMarkOffset;
    int previousMarkPassedNumber;

    static void IRAM_ATTR startRobotWithBootButton(void *arg);
    void configExternInterruptToReadButton(gpio_num_t gpio_num);
    void startFollowingDefinedMapping();
    void defineIfRobotWillStartMappingMode();
    void waitPressBootButtonToStart();
    void deleteMappingIfBootButtonIsPressed();
    void startMappingTheTrack();
    void setTuningMode();
    bool passedFirstMark();
    void resetEnconderInFirstMark();
    bool trackSegmentChanged();
    bool RobotStateChanged();
    LedColor defineLedColor();
    void setColorBrightness(LedColor color);
    void logCarStatus();
    void stopTunningMode();
    void defineTrackSegment(MapData Mark);
    void UpdateMarkPassedNumber(int markNumber);
    TrackSegment getTrackSegment(MapData Mark);
};

#endif