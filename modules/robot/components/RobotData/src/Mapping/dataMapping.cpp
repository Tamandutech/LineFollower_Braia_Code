#include "dataMapping.h"

#include "MappingService.hpp"

dataMapping::dataMapping(std::string name)
{
    this->name = name;
    ESP_LOGD(name.c_str(), "Criando objeto: %s (%p)", name.c_str(), this);

    dataManager = dataManager->getInstance();

    latEsqPass = new DataAbstract<bool>("latEsqPass", name, 0);
    latDirPass = new DataAbstract<bool>("latDirPass", name, 0);

    leftMarks = new DataAbstract<uint16_t>("leftMarks", name, 0);
    rightMarks = new DataAbstract<uint16_t>("rightMarks", name, 0);

    TrackSideMarks = new DataMap("TrackSideMarks", name);

    thresholdToCurve = new DataAbstract<uint8_t>("thresholdToCurve", name, 25);
    dataManager->registerParamData(thresholdToCurve);

    MarkstoStop = new DataAbstract<uint16_t>("MarkstoStop", name, 2);
    dataManager->registerParamData(MarkstoStop);

    PulsesBeforeCurve = new DataAbstract<uint32_t>("PulsesBeforeCurve", name, 200);
    dataManager->registerParamData(PulsesBeforeCurve);

    targetNumberSensorReadsToMean = new DataAbstract<uint16_t>("targetNumberSensorReadsToMean", name, 6);
    dataManager->registerParamData(targetNumberSensorReadsToMean);

    LongLineLength = new DataAbstract<uint16_t>("LongLine_Length", name, 1500);
    dataManager->registerParamData(LongLineLength);
    
    MediumLineLength = new DataAbstract<uint16_t>("MediumLine_Length", name, 1000);
    dataManager->registerParamData(MediumLineLength);

    LongCurveLength = new DataAbstract<uint16_t>("LongCurve_Length", name, 900);
    dataManager->registerParamData(LongCurveLength);
    
    MediumCurveLength = new DataAbstract<uint16_t>("MediumCurve_Length", name, 500);
    dataManager->registerParamData(MediumCurveLength);

}

void dataMapping::leftPassedInc()
{
    this->leftMarks->setData(this->leftMarks->getData() + 1);

    MappingService::getInstance()->createNewMark();
}

void dataMapping::rightPassedInc()
{
    this->rightMarks->setData(this->rightMarks->getData() + 1);
    if(this->rightMarks->getData() <= 1 || this->rightMarks->getData() == this->MarkstoStop->getData()) MappingService::getInstance()->createNewMark();
}