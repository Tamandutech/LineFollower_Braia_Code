#include <map>
#include <list>
#include <algorithm>

#include "RobotData.h"

auto pid = Robot::getInstance()->getPID();
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
    {SPECIAL_TRACK, speed->Special_Track->getData()}};

float getTrackStatusSpeed(TrackState trackState)
{
    bool trackStatusFound = TrackStatusSpeed.find(trackState) != TrackStatusSpeed.end();

    if (trackStatusFound)
        return TrackStatusSpeed[trackState];

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

std::map<TypePID, PID> TrackStatusPID{
    {LINE, PID(
               pid->Kp_line->getData(),
               pid->Ki_default->getData(),
               pid->Kd_line->getData())},
    {CURVE, PID(
                pid->Kp_curve->getData(),
                pid->Ki_default->getData(),
                pid->Kd_curve->getData())},
    {ZIGZAG, PID(
                 pid->Kp_ZigZag->getData(),
                 pid->Ki_default->getData(),
                 pid->Kd_ZigZag->getData())},
    {TUNNING, PID(
                  pid->Kp_tunning->getData(),
                  pid->Ki_tunning->getData(),
                  pid->Kd_tunning->getData())},
    {DEFAULT, PID(
                  pid->Kp_default->getData(),
                  pid->Ki_default->getData(),
                  pid->Kd_default->getData())},
};

PID getTrackStatusPID(TrackState trackState, CarState status)
{
    TypePID type = getTypePID(trackState, status);
    return TrackStatusPID[type];
}

TypePID getTypePID(TrackState trackState, CarState status)
{

    std::list<TrackState> line = {SHORT_LINE, MEDIUM_LINE, LONG_LINE, XLONG_LINE};
    std::list<TrackState> curve = {SHORT_CURVE, MEDIUM_CURVE, LONG_CURVE, XLONG_CURVE};

    bool trackStateIsLine = (std::find(line.begin(), line.end(), trackState) != line.end());
    bool trackStateIsCurve = (std::find(curve.begin(), curve.end(), trackState) != curve.end());

    TypePID type = DEFAULT;

    if (status == CAR_TUNING)
        type = TUNNING;
    else if (trackStateIsLine)
        type = LINE;
    else if (trackStateIsCurve)
        type = CURVE;
    else if (trackState == ZIGZAG)
        type = ZIGZAG;
}