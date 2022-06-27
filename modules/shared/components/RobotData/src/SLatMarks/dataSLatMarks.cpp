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

    MarkstoStop = new DataAbstract<uint8_t>("MarkstoStop", name, 2);
    dataManager->registerParamData(MarkstoStop);

    PulsesBeforeCurve = new DataAbstract<uint32_t>("PulsesBeforeCurve", name, 200);
    dataManager->registerParamData(PulsesBeforeCurve);
}

void dataSLatMarks::leftPassedInc()
{
    this->leftMarks->setData(this->leftMarks->getData() + 1);

    MappingService::getInstance()->createNewMark();
}

void dataSLatMarks::rightPassedInc()
{
    this->rightMarks->setData(this->rightMarks->getData() + 1);

    MappingService::getInstance()->createNewMark();
}