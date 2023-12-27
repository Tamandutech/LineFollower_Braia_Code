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

enum TypePID
{
    LINE = 1,
    CURVE = 2,
    ZIGZAG = 3,
    TUNNING = 4,
    DEFAULT = 5
};

struct PID
{
    double KP;
    double KI;
    double KD;

    PID(double kp, double ki, double kd) : KP(kp), KI(ki), KD(kd){};
};

std::map<TypePID, PID> TrackSegmentPID{
    {TypePID::LINE, PID(pid->Kp_line->getData(), pid->Ki_default->getData(), pid->Kd_line->getData())},
    {TypePID::CURVE, PID(pid->Kp_curve->getData(), pid->Ki_default->getData(), pid->Kd_curve->getData())},
    {TypePID::ZIGZAG, PID(pid->Kp_ZigZag->getData(), pid->Ki_default->getData(), pid->Kd_ZigZag->getData())},
    {TypePID::TUNNING, PID(pid->Kp_tunning->getData(), pid->Ki_tunning->getData(), pid->Kd_tunning->getData())},
    {TypePID::DEFAULT, PID(pid->Kp_default->getData(), pid->Ki_default->getData(), pid->Kd_default->getData())},
};

PID getTrackSegmentPID(TrackSegment TrackSegment, CarState status)
{
    TypePID type = getTypePID(TrackSegment, status);
    return TrackSegmentPID[type];
}

TypePID getTypePID(TrackSegment trackSegment, CarState status)
{

    const std::list<TrackSegment> line = {SHORT_LINE, MEDIUM_LINE, LONG_LINE, XLONG_LINE};
    const std::list<TrackSegment> curve = {SHORT_CURVE, MEDIUM_CURVE, LONG_CURVE, XLONG_CURVE};

    TypePID type = DEFAULT;

    if (status == CAR_TUNING)
        type = TUNNING;
    else if (trackSegment == ZIGZAG)
        type = ZIGZAG;
    else if (existTrackSegmentInList(line, trackSegment))
        type = LINE;
    else if (existTrackSegmentInList(curve, trackSegment))
        type = CURVE;

    return type;
}

bool existTrackSegmentInList(std::list<TrackSegment> list, TrackSegment trackSegment) {
    return (std::find(list.begin(), list.end(), trackSegment) != list.end());
}

