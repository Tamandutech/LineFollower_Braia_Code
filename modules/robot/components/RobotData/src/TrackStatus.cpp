#include <map>
#include <list>
#include <algorithm>

#include "RobotData.h"

auto speed = Robot::getInstance()->getSpeed();

enum TrackState
{
    SHORT_LINE = 1,
    MEDIUM_LINE = 2,
    LONG_LINE = 3,
    XLONG_LINE = 4,

    SHORT_CURVE = 5,
    MEDIUM_CURVE = 6,
    LONG_CURVE = 7,
    XLONG_CURVE = 8,

    ZIGZAG = 9,
    SPECIAL_TRACK = 10,
    DEFAULT_TRACK = 11
};

std::map<TrackState, float> TrackStatusSpeed{
    {SHORT_LINE, speed->Short_Line->getData()},
    {MEDIUM_LINE, speed->Medium_Line->getData()},
    {LONG_LINE, speed->Long_Line->getData()},
    {XLONG_LINE, speed->XLong_Line->getData()},

    {SHORT_CURVE, speed->Short_Curve->getData()},
    {MEDIUM_CURVE, speed->Medium_Curve->getData()},
    {LONG_CURVE, speed->Long_Curve->getData()},
    {XLONG_CURVE, speed->XLong_Curve->getData()},

    {ZIGZAG, speed->Zig_Zag->getData()},
    {SPECIAL_TRACK, speed->Special_Track->getData()}
};

float getTrackStatusSpeed(TrackState trackState)
{
    bool trackStatusFound = TrackStatusSpeed.find(trackState) != TrackStatusSpeed.end();

    if (trackStatusFound)
        return TrackStatusSpeed[trackState];

    return speed->Default_speed->getData();
}
