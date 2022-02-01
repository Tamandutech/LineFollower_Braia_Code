#include "dataSLatMarks.h"

dataSLatMarks::dataSLatMarks(std::string name){
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);
    ESP_LOGD(tag, "Criando Semáforos");
    vSemaphoreCreateBinary(xSemaphorelatesqPass);
    vSemaphoreCreateBinary(xSemaphorelatdirPass);
    vSemaphoreCreateBinary(xSemaphoreleftMarks);
    vSemaphoreCreateBinary(xSemaphorerightMarks);
    vSemaphoreCreateBinary(xSemaphoreMarksData);
    vSemaphoreCreateBinary(xSemaphoreTotalLeftMarks);
    vSemaphoreCreateBinary(xSemaphoreInitialMark);
    vSemaphoreCreateBinary(xSemaphoreFinalMark);
    vSemaphoreCreateBinary(xSemaphoreMapFinished);
}
bool dataSLatMarks::getSLatEsq(){
    bool tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphorelatesqPass, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->latesqPass;
            xSemaphoreGive(xSemaphorelatesqPass);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável latesqPass ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
int dataSLatMarks::SetSLatEsq(bool latesqPass)
{
    if (xSemaphoreTake(xSemaphorelatesqPass, (TickType_t)10) == pdTRUE)
    {
        this->latesqPass = latesqPass;
        xSemaphoreGive(xSemaphorelatesqPass);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável latesqPass ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
bool dataSLatMarks::getSLatDir(){
    bool tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphorelatdirPass, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->latdirPass;
            xSemaphoreGive(xSemaphorelatdirPass);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável latdirPass ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
int dataSLatMarks::SetSLatDir(bool latdirPass)
{
    if (xSemaphoreTake(xSemaphorelatdirPass, (TickType_t)10) == pdTRUE)
    {
        this->latdirPass = latdirPass;
        xSemaphoreGive(xSemaphorelatdirPass);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável latdirPass ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int dataSLatMarks::leftPassedInc()
{
    if (xSemaphoreTake(xSemaphoreleftMarks, (TickType_t)10) == pdTRUE)
    {
        this->leftMarks++;
        xSemaphoreGive(xSemaphoreleftMarks);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável leftMarks ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int dataSLatMarks::rightPassedInc()
{
    if (xSemaphoreTake(xSemaphorerightMarks, (TickType_t)10) == pdTRUE)
    {
        this->rightMarks++;
        xSemaphoreGive(xSemaphorerightMarks);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável rightMarks ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
struct MapData dataSLatMarks::getMarkDataReg(int regnum){
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
int dataSLatMarks::SetMarkDataReg(struct MapData markreg, int regnum){
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
uint16_t dataSLatMarks::getleftMarks(){
    uint16_t tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreleftMarks, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->leftMarks;
            xSemaphoreGive(xSemaphoreleftMarks);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável leftMarks ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
uint16_t dataSLatMarks::getrightMarks(){
    uint16_t tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphorerightMarks, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->rightMarks;
            xSemaphoreGive(xSemaphorerightMarks);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável rightMarks ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
uint16_t dataSLatMarks::getTotalLeftMarks(){
    uint16_t tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreTotalLeftMarks, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->TotalLeftMarks;
            xSemaphoreGive(xSemaphoreTotalLeftMarks);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável TotalLeftMarks ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
int dataSLatMarks::SetTotalLeftMarks(uint16_t totalmarks)
{
    if (xSemaphoreTake(xSemaphoreTotalLeftMarks, (TickType_t)10) == pdTRUE)
    {
        this->TotalLeftMarks = totalmarks;
        xSemaphoreGive(xSemaphoreTotalLeftMarks);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável TotalLeftMarks ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int32_t dataSLatMarks::getInitialMark(){
    int32_t tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreInitialMark, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->InitialMark;
            xSemaphoreGive(xSemaphoreInitialMark);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável InitialMark ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
int dataSLatMarks::SetInitialMark(int32_t initialmark)
{
    if (xSemaphoreTake(xSemaphoreInitialMark, (TickType_t)10) == pdTRUE)
    {
        this->InitialMark = initialmark;
        xSemaphoreGive(xSemaphoreInitialMark);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável InitialMark ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int32_t dataSLatMarks::getFinalMark(){
    int32_t tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreFinalMark, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->FinalMark;
            xSemaphoreGive(xSemaphoreFinalMark);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável FinalMark ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
int dataSLatMarks::SetFinalMark(int32_t finalmark)
{
    if (xSemaphoreTake(xSemaphoreFinalMark, (TickType_t)10) == pdTRUE)
    {
        this->FinalMark = finalmark;
        xSemaphoreGive(xSemaphoreFinalMark);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável FinalMark ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
bool dataSLatMarks::getMapFinished(){
    bool tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreMapFinished, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->MapFinished;
            xSemaphoreGive(xSemaphoreMapFinished);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável MapFinished ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}
int dataSLatMarks::SetMapFinished(bool mapfinished)
{
    if (xSemaphoreTake(xSemaphoreMapFinished, (TickType_t)10) == pdTRUE)
    {
        this->MapFinished = mapfinished;
        xSemaphoreGive(xSemaphoreMapFinished);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável MapFinished ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}