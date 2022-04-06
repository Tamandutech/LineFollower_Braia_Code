#include "dataSLatMarks.h"

dataSLatMarks::dataSLatMarks(std::string name)
{
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    ESP_LOGD(tag, "Criando Semáforos");
    (xSemaphoreMarksData) = xSemaphoreCreateMutex();

    latEsqPass = new DataAbstract<bool>("latEsqPass", 0);
    latDirPass = new DataAbstract<bool>("latDirPass", 0);
    leftMarks = new DataAbstract<uint16_t>("leftMarks", 0);
    rightMarks = new DataAbstract<uint16_t>("rightMarks", 0);
    totalLeftMarks = new DataAbstract<uint16_t>("totalLeftMarks", 0);
    initialMark = new DataAbstract<int32_t>("initialMark", 0);
    finalMark = new DataAbstract<int32_t>("finalMark", 0);
    mapFinished = new DataAbstract<bool>("mapFinished", 0);
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
    struct MapData tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreMarksData, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->MarksData[regnum];
            xSemaphoreGive(xSemaphoreMarksData);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável MarksData ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
int dataSLatMarks::SetMarkDataReg(struct MapData markreg, int regnum)
{
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreMarksData, (TickType_t)10) == pdTRUE)
        {
            this->MarksData[regnum] = markreg;
            xSemaphoreGive(xSemaphoreMarksData);
            return RETORNO_OK;
        }
        else
        {
            ESP_LOGE(tag, "Variável MarksData ocupada, não foi possível ler valor. Tentando novamente...");
            return RETORNO_VARIAVEL_OCUPADA;
        }
    }
}

struct SLatMarks dataSLatMarks::getData()
{
    struct SLatMarks SLatData;

    SLatData.leftMarks = this->leftMarks->getData();
    SLatData.rightMarks = this->rightMarks->getData();
    SLatData.InitialMark = this->initialMark->getData();
    SLatData.FinalMark = this->finalMark->getData();
    SLatData.TotalLeftMarks = this->totalLeftMarks->getData();
    SLatData.MapFinished = this->mapFinished->getData();

    if (xSemaphoreTake(xSemaphoreMarksData, (TickType_t)10) == pdTRUE)
    {
        memcpy(SLatData.MarksData, this->MarksData, sizeof(this->MarksData));
        xSemaphoreGive(xSemaphoreMarksData);
    }
    else
    {
        ESP_LOGE(tag, "Variável MarksData ocupada, não foi possível ler valor. Tentando novamente...");
    }
    return SLatData;
}
int dataSLatMarks::setData(struct SLatMarks SLatData)
{
    this->initialMark->setData(SLatData.InitialMark);
    this->finalMark->setData(SLatData.FinalMark);
    this->totalLeftMarks->setData(SLatData.TotalLeftMarks);
    this->mapFinished->setData(SLatData.MapFinished);

    if (xSemaphoreTake(xSemaphoreMarksData, (TickType_t)10) == pdTRUE)
    {
        memcpy(this->MarksData, SLatData.MarksData, sizeof(this->MarksData));
        xSemaphoreGive(xSemaphoreMarksData);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável MarksData ocupada, não foi possível ler valor. Tentando novamente...");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}