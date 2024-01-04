#ifndef TYPE_PID_HPP
#define TYPE_PID_HPP

#include <map>
#include <list>
#include <algorithm>

#include "dataPID.h"
#include "TrackSegment.hpp"

enum TypePID
{
    LINE = 1,
    CURVE = 2,
    ZIGZAG = 3,
    TUNNING = 4,
    DEFAULT = 5
};

struct PID_Consts
{
    double KP;
    double KI;
    double KD;
};

TypePID getTypePID(TrackSegment trackSegment, CarState status);
PID_Consts getTrackSegmentPID(TrackSegment TrackSegment, CarState status, dataPID *pid);
std::map<TypePID, PID_Consts> getPIDValueFromDashboard(dataPID *pid);
#endif


