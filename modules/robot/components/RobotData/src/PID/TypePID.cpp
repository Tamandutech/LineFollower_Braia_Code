#include "TypePID.hpp"

std::map<TypePID, PID_Consts> getPIDValueFromDashboard(dataPID *pid)
{
    std::map<TypePID, PID_Consts> TrackSegmentPID{
        {TypePID::LINE, {pid->Kp_line->getData(), pid->Ki_default->getData(), pid->Kd_line->getData()}},
        {TypePID::CURVE, {pid->Kp_curve->getData(), pid->Ki_default->getData(), pid->Kd_curve->getData()}},
        {TypePID::ZIGZAG, {pid->Kp_ZigZag->getData(), pid->Ki_default->getData(), pid->Kd_ZigZag->getData()}},
        {TypePID::TUNNING, {pid->Kp_tunning->getData(), pid->Ki_tunning->getData(), pid->Kd_tunning->getData()}},
        {TypePID::DEFAULT, {pid->Kp_default->getData(), pid->Ki_default->getData(), pid->Kd_default->getData()}},
    };
    return TrackSegmentPID;
}

PID_Consts getTrackSegmentPID(TrackSegment TrackSegment, CarState status, dataPID *pid)
{
    std::map<TypePID, PID_Consts> TrackSegmentPID = getPIDValueFromDashboard(pid);
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
    else if (trackSegment == ZIGZAG_TRACK)
        type = ZIGZAG;
    else if (existTrackSegmentInList(line, trackSegment))
        type = LINE;
    else if (existTrackSegmentInList(curve, trackSegment))
        type = CURVE;

    return type;
}