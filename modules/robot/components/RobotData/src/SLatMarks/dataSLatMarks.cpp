#include "dataSLatMarks.h"

#include "MappingService.hpp"

dataSLatMarks::dataSLatMarks(std::string name)
{
    this->name = name;
    ESP_LOGD(name.c_str(), "Criando objeto: %s (%p)", name.c_str(), this);

    dataManager = dataManager->getInstance();

    latEsqPass = new DataAbstract<bool>("latEsqPass", name, 0);
    latDirPass = new DataAbstract<bool>("latDirPass", name, 0);

    leftMarks = new DataAbstract<uint16_t>("leftMarks", name, 0);
    rightMarks = new DataAbstract<uint16_t>("rightMarks", name, 0);

    marks = new DataMap("marks", name);
    dataManager->registerParamData(marks);

    thresholdToCurve = new DataAbstract<uint8_t>("thresholdToCurve", name, 25);
    dataManager->registerParamData(thresholdToCurve);

    MarkstoStop = new DataAbstract<uint16_t>("MarkstoStop", name, 2);
    dataManager->registerParamData(MarkstoStop);

    PulsesBeforeCurve = new DataAbstract<uint32_t>("PulsesBeforeCurve", name, 200);
    dataManager->registerParamData(PulsesBeforeCurve);

    MarkstoMean = new DataAbstract<uint16_t>("MarkstoMean", name, 6);
    dataManager->registerParamData(MarkstoMean);

    PulsesAfterCurve = new DataAbstract<uint32_t>("PulsesAfterCurve", name, 200);
    dataManager->registerParamData(PulsesAfterCurve);

    thresholdLongLine = new DataAbstract<uint16_t>("thresholdLongLine", name, 1500);
    dataManager->registerParamData(thresholdLongLine);
    
    thresholdMediumLine = new DataAbstract<uint16_t>("thresholdMediumLine", name, 1000);
    dataManager->registerParamData(thresholdMediumLine);

    thresholdLongCurve = new DataAbstract<uint16_t>("thresholdLongCurve", name, 900);
    dataManager->registerParamData(thresholdLongCurve);
    
    thresholdMediumCurve = new DataAbstract<uint16_t>("thresholdMediumCurve", name, 500);
    dataManager->registerParamData(thresholdMediumCurve);

}

void dataSLatMarks::leftPassedInc()
{
    this->leftMarks->setData(this->leftMarks->getData() + 1);

    MappingService::getInstance()->createNewMark();
}

void dataSLatMarks::rightPassedInc()
{
    this->rightMarks->setData(this->rightMarks->getData() + 1);
    if(this->rightMarks->getData() <= 1 || this->rightMarks->getData() == this->MarkstoStop->getData()) MappingService::getInstance()->createNewMark();
}