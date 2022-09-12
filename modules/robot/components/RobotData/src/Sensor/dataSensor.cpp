#include "dataSensor.h"

dataSensor::dataSensor(uint16_t qtdChannels, std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    this->qtdChannels = qtdChannels;
    // Alocando espaço para as variáveis
    ESP_LOGD(tag, "Alocando espaço na memória paras as variáveis, quantidade de canais: %d", qtdChannels);
    channel.reserve(qtdChannels);
    maxChannel.reserve(qtdChannels);
    minChannel.reserve(qtdChannels);

    ESP_LOGD(tag, "Criando Semáforos");
    (xSemaphorechannel) = xSemaphoreCreateMutex();
    (xSemaphoreline) = xSemaphoreCreateMutex();
    (xSemaphoremaxChannel) = xSemaphoreCreateMutex();
    (xSemaphoreminChannel) = xSemaphoreCreateMutex();
}

int dataSensor::setLine(uint16_t value)
{
    if (xSemaphoreTake(xSemaphoreline, (TickType_t)10) == pdTRUE)
    {
        this->line = value;
        xSemaphoreGive(xSemaphoreline);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável Line ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
uint16_t dataSensor::getLine()
{
    int16_t tempLine = 0;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreline, (TickType_t)10) == pdTRUE)
        {
            tempLine = this->line;
            xSemaphoreGive(xSemaphoreline);
            return tempLine;
        }
        else
        {
            ESP_LOGE(tag, "Variável Output ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}

int dataSensor::setChannel(uint16_t channelNumber, uint16_t value, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg)
{
    if (channelNumber > (qtdChannels - 1))
    {
        ESP_LOGE(tag, "O canal informado \"%ud\" não existe, máximo: %ud", channelNumber, (qtdChannels - 1));
        return RETORNO_ARGUMENTO_INVALIDO;
    }

    if (xSemaphoreTake((*xSemaphoreOfArg), (TickType_t)10) == pdTRUE)
    {
        (*channel)[channelNumber] = value;
        xSemaphoreGive((*xSemaphoreOfArg));
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Vetor de canais ocupado, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int dataSensor::setChannel(uint16_t channelNumber, uint16_t value)
{
    return this->setChannel(channelNumber, value, &this->channel, &xSemaphorechannel);
}

uint16_t dataSensor::getChannel(uint16_t channelNumber, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg)
{
    if (channelNumber > (qtdChannels - 1))
    {
        ESP_LOGE(tag, "O canal informado \"%ud\" não existe, máximo: %ud", channelNumber, (qtdChannels - 1));
        return RETORNO_ARGUMENTO_INVALIDO;
    }

    uint16_t tempChannel;
    for (;;)
    {
        if (xSemaphoreTake((*xSemaphoreOfArg), (TickType_t)10) == pdTRUE)
        {
            tempChannel = (*channel)[channelNumber];
            xSemaphoreGive((*xSemaphoreOfArg));
            return tempChannel;
        }
        else
        {
            ESP_LOGE(tag, "Vetor de canais ocupado, não foi possível ler valor. Tentando novamente...");
        }
    }
}
uint16_t dataSensor::getChannel(uint16_t channelNumber)
{
    return this->getChannel(channelNumber, &this->channel, &xSemaphorechannel);
}

int dataSensor::setChannels(std::vector<uint16_t> values, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg)
{
    if (xSemaphoreTake((*xSemaphoreOfArg), (TickType_t)10) == pdTRUE)
    {
        (*channel) = values;
        xSemaphoreGive((*xSemaphoreOfArg));
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Vetor de canais ocupado, não foi possível definir valores.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int dataSensor::setChannels(std::vector<uint16_t> values)
{
    return this->setChannels(values, &this->channel, &xSemaphorechannel);
}

std::vector<uint16_t> dataSensor::getChannels(std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg)
{
    std::vector<uint16_t> tempChannels;
    for (;;)
    {
        if (xSemaphoreTake((*xSemaphoreOfArg), (TickType_t)10) == pdTRUE)
        {
            tempChannels = (*channel);
            xSemaphoreGive((*xSemaphoreOfArg));
        }
        else
        {
            ESP_LOGE(tag, "Vetor de canais ocupado, não foi possível ler valores. Tentando novamente...");
        }

        return tempChannels;
    }
}
std::vector<uint16_t> dataSensor::getChannels()
{
    return this->getChannels(&this->channel, &xSemaphorechannel);
}

int dataSensor::setChannelMin(uint16_t channelNumber, uint16_t value)
{
    return this->setChannel(channelNumber, value, &this->minChannel, &xSemaphoreminChannel);
}
uint16_t dataSensor::getChannelMin(uint16_t channelNumber)
{
    return this->getChannel(channelNumber, &this->minChannel, &xSemaphoreminChannel);
}

int dataSensor::setChannelMax(uint16_t channelNumber, uint16_t value)
{
    return this->setChannel(channelNumber, value, &this->maxChannel, &xSemaphoremaxChannel);
}
uint16_t dataSensor::getChannelMax(uint16_t channelNumber)
{
    return this->getChannel(channelNumber, &this->maxChannel, &xSemaphoremaxChannel);
}

int dataSensor::setChannelsMins(std::vector<uint16_t> values)
{
    return this->setChannels(values, &this->minChannel, &xSemaphoreminChannel);
}
std::vector<uint16_t> dataSensor::getChannelsMins()
{
    return this->getChannels(&this->minChannel, &xSemaphoreminChannel);
}

int dataSensor::setChannelsMaxes(std::vector<uint16_t> values)
{
    return this->setChannels(values, &this->maxChannel, &xSemaphoremaxChannel);
}
std::vector<uint16_t> dataSensor::getChannelsMaxes()
{
    return this->getChannels(&this->maxChannel, &xSemaphoremaxChannel);
}