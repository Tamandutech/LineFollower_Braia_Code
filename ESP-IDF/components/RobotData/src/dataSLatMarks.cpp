#include "dataSLatMarks.h"

dataSLatMarks::dataSLatMarks(std::string name){
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);
    ESP_LOGD(tag, "Criando Semáforos");
    vSemaphoreCreateBinary(xSemaphorelatesqPass);
    vSemaphoreCreateBinary(xSemaphorelatdirPass);
    vSemaphoreCreateBinary(xSemaphoreleftMarks);
    vSemaphoreCreateBinary(xSemaphorerightMarks);
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
            tempInput = this->latesqPass;
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
        ESP_LOGE(tag, "Variável leftMarks ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
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