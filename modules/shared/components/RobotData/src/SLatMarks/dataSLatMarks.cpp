#include "dataSLatMarks.h"

dataSLatMarks::dataSLatMarks(std::string name)
{
    this->name = name;
    ESP_LOGD(name.c_str(), "Criando objeto: %s (%p)", name.c_str(), this);

    dataManager = dataManager->getInstance();

    ESP_LOGD(tag, "Criando Semáforos");
    // (xSemaphoreMarksData) = xSemaphoreCreateMutex();

    latEsqPass = new DataAbstract<bool>("latEsqPass", name, 0);
    latDirPass = new DataAbstract<bool>("latDirPass", name, 0);
    leftMarks = new DataAbstract<uint16_t>("leftMarks", name, 0);
    rightMarks = new DataAbstract<uint16_t>("rightMarks", name, 0);
    totalLeftMarks = new DataAbstract<uint16_t>("totalLeftMarks", name, 0);
    finalMark = new DataAbstract<int32_t>("finalMark", name, 0);
    mapFinished = new DataAbstract<bool>("mapFinished", name, 0);

    ESP_LOGD(tag, "Criando objetos do tipo DataMap");

    marks = new DataMap("marks", name);
    dataManager->registerParamData(marks);
}

void dataSLatMarks::leftPassedInc()
{
    this->leftMarks->setData(this->leftMarks->getData() + 1);
}
void dataSLatMarks::rightPassedInc()
{
    this->rightMarks->setData(this->rightMarks->getData() + 1);
}

struct MapData dataSLatMarks::getMarkDataReg(int regnum)
{
    return marks->getData(regnum);
}

int dataSLatMarks::SetMarkDataReg(struct MapData markreg, int regnum)
{
    marks->setData(regnum, markreg);

    return 0;
}

// struct SLatMarks dataSLatMarks::getData()
// {
//     struct SLatMarks SLatData;

//     SLatData.leftMarks = this->leftMarks->getData();
//     SLatData.rightMarks = this->rightMarks->getData();

//     SLatData.TotalLeftMarks = this->totalLeftMarks->getData();

//     SLatData.FinalMark = this->finalMark->getData(); // média dos enconders para o final da ultima marcação direita
//     SLatData.MapFinished = this->mapFinished->getData();

//     if (xSemaphoreTake(xSemaphoreMarksData, (TickType_t)10) == pdTRUE)
//     {
//         memcpy(SLatData.MarksData, this->MarksData, sizeof(this->MarksData));
//         xSemaphoreGive(xSemaphoreMarksData);
//     }
//     else
//     {
//         ESP_LOGE(tag, "Variável MarksData ocupada, não foi possível ler valor. Tentando novamente...");
//     }
//     return SLatData;
// }

// int dataSLatMarks::setData(struct SLatMarks SLatData)
// {
//     this->finalMark->setData(SLatData.FinalMark);
//     this->totalLeftMarks->setData(SLatData.TotalLeftMarks);
//     this->mapFinished->setData(SLatData.MapFinished);

//     if (xSemaphoreTake(xSemaphoreMarksData, (TickType_t)10) == pdTRUE)
//     {
//         memcpy(this->MarksData, SLatData.MarksData, sizeof(this->MarksData));
//         xSemaphoreGive(xSemaphoreMarksData);
//         return RETORNO_OK;
//     }
//     else
//     {
//         ESP_LOGE(tag, "Variável MarksData ocupada, não foi possível ler valor. Tentando novamente...");
//         return RETORNO_VARIAVEL_OCUPADA;
//     }
// }