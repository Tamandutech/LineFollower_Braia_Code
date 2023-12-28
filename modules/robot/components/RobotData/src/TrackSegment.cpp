#include <map>
#include <list>
#include <algorithm>

#include "RobotData.h"

auto pid = Robot::getInstance()->getPID();
auto speed = Robot::getInstance()->getSpeed();

enum TrackSegment
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

std::map<TrackSegment, float> TrackSegmentSpeed{
    {TrackSegment::SHORT_LINE, speed->Short_Line->getData()},
    {TrackSegment::MEDIUM_LINE, speed->Medium_Line->getData()},
    {TrackSegment::LONG_LINE, speed->Long_Line->getData()},
    {TrackSegment::XLONG_LINE, speed->XLong_Line->getData()},

    {TrackSegment::SHORT_CURVE, speed->Short_Curve->getData()},
    {TrackSegment::MEDIUM_CURVE, speed->Medium_Curve->getData()},
    {TrackSegment::LONG_CURVE, speed->Long_Curve->getData()},
    {TrackSegment::XLONG_CURVE, speed->XLong_Curve->getData()},

    {TrackSegment::ZIGZAG, speed->ZIGZAG->getData()},
    {TrackSegment::SPECIAL_TRACK, speed->Special_Track->getData()}
};

float getTrackSegmentSpeed(TrackSegment trackSegment)
{
    bool trackStatusFound = TrackSegmentSpeed.find(trackSegment) != TrackSegmentSpeed.end();

    if (trackStatusFound)
        return TrackSegmentSpeed[trackSegment];

    return speed->Default_speed->getData();
}



