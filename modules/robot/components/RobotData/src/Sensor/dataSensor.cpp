#include "dataSensor.h"

dataSensor::dataSensor(uint16_t sensorNumber, std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s (%p)", name.c_str(), this);

    this->sensorNumber = sensorNumber;
    // Alocando espaço para as variáveis
    ESP_LOGD(tag, "Alocando espaço na memória paras as variáveis, quantidade de canais: %d", sensorNumber);
    channel.reserve(sensorNumber);

    ESP_LOGD(tag, "Criando Semáforos");
    (xSemaphorechannel) = xSemaphoreCreateMutex();
    (xSemaphoreline) = xSemaphoreCreateMutex();
}

int dataSensor::setLine(uint16_t value)
{
    if (isSensorAvailableForReading(&xSemaphoreline))
    {
        this->line = value;
        xSemaphoreGive(xSemaphoreline);
        return RETORNO_OK;
    }

    ESP_LOGE(tag, "Variável Line ocupada, não foi possível definir valor.");
    return RETORNO_VARIAVEL_OCUPADA;
}

uint16_t dataSensor::getLine()
{
    if (isSensorAvailableForReading(&xSemaphoreline))
    {
        int16_t tempLine = this->line;
        xSemaphoreGive(xSemaphoreline);
        return tempLine;
    }

    ESP_LOGE(tag, "Variável Output ocupada, não foi possível ler valor. Tentando novamente...");
}

// TODO: Verificar se tem uma forma melhor de fazer essas tratativas
int dataSensor::setChannel(uint16_t sensorPosition, uint16_t value, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg)
{
    ESP_ERROR_CHECK(checksIfSensorExisty(sensorPosition));

    if (isSensorAvailableForReading(xSemaphoreOfArg))
    {
        (*channel)[sensorPosition] = value;
        xSemaphoreGive((*xSemaphoreOfArg));
        return RETORNO_OK;
    }

    ESP_LOGE(tag, "Vetor de canais ocupado, não foi possível definir valor.");
    return RETORNO_VARIAVEL_OCUPADA;
}

uint16_t dataSensor::getChannel(uint16_t sensorPosition, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg)
{
    ESP_ERROR_CHECK(checksIfSensorExisty(sensorPosition));

    if (isSensorAvailableForReading(xSemaphoreOfArg))
    {
        uint16_t valueSensoReading = (*channel)[sensorPosition];
        xSemaphoreGive((*xSemaphoreOfArg));
        return valueSensoReading;
    }

    ESP_LOGE(tag, "Vetor de canais ocupado, não foi possível ler valor. Tentando novamente...");
}

uint16_t dataSensor::getChannel(uint16_t sensorPosition)
{
    return this->getChannel(sensorPosition, &this->channel, &xSemaphorechannel);
}

int dataSensor::setChannel(uint16_t sensorPosition, uint16_t value)
{
    return this->setChannel(sensorPosition, value, &this->channel, &xSemaphorechannel);
}

std::vector<uint16_t> dataSensor::getChannels()
{
    return this->getChannels(&this->channel, &xSemaphorechannel);
}

int dataSensor::setChannels(std::vector<uint16_t> values)
{
    return this->setChannels(values, &this->channel, &xSemaphorechannel);
}

std::vector<uint16_t> dataSensor::getChannels(std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg)
{
    std::vector<uint16_t> tempChannels;
    if (isSensorAvailableForReading(xSemaphoreOfArg))
    {
        tempChannels = (*channel);
        xSemaphoreGive((*xSemaphoreOfArg));
    }

    ESP_LOGE(tag, "Vetor de canais ocupado, não foi possível ler valores. Tentando novamente...");
    return tempChannels;
}

int dataSensor::setChannels(std::vector<uint16_t> values, std::vector<uint16_t> *channel, SemaphoreHandle_t *xSemaphoreOfArg)
{
    if (isSensorAvailableForReading(xSemaphoreOfArg))
    {
        (*channel) = values;
        xSemaphoreGive((*xSemaphoreOfArg));
        return RETORNO_OK;
    }

    ESP_LOGE(tag, "Vetor de canais ocupado, não foi possível definir valores.");
    return RETORNO_VARIAVEL_OCUPADA;
}

esp_err_t dataSensor::checksIfSensorExisty(uint16_t sensorPosition)
{
    if (sensorPosition > (this->sensorNumber - 1))
    {
        ESP_LOGE(tag, "O canal informado \"%ud\" não existe, máximo: %ud", sensorPosition, (sensorNumber - 1));
        return ESP_FAIL;
    }
    return ESP_OK;
}

bool dataSensor::isSensorAvailableForReading(SemaphoreHandle_t *xSemaphoreOfArg)
{
    return xSemaphoreTake((*xSemaphoreOfArg), portMAX_DELAY);
}