#include "dataPID.h"

dataPID::dataPID(std::string name)
{
    // Definindo nome do objeto, para uso nas logs do componente.
    this->name = name;
    ESP_LOGD(tag, "Criando objeto: %s", name.c_str());

    ESP_LOGD(tag, "Criando Semáforos");
    vSemaphoreCreateBinary(xSemaphoreInput);
    vSemaphoreCreateBinary(xSemaphoreOutput);
    vSemaphoreCreateBinary(xSemaphoreSetpoint);
    vSemaphoreCreateBinary(xSemaphoreKp_line);
    vSemaphoreCreateBinary(xSemaphoreKp_curve);
    vSemaphoreCreateBinary(xSemaphoreKi_line);
    vSemaphoreCreateBinary(xSemaphoreKi_curve);
    vSemaphoreCreateBinary(xSemaphoreKd_line);
    vSemaphoreCreateBinary(xSemaphoreKd_curve);
}

int dataPID::setInput(int16_t input)
{
    if (xSemaphoreTake(xSemaphoreInput, (TickType_t)10) == pdTRUE)
    {
        this->input = input;
        xSemaphoreGive(xSemaphoreInput);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável Input ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int16_t dataPID::getInput()
{
    int16_t tempInput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreInput, (TickType_t)10) == pdTRUE)
        {
            tempInput = this->input;
            xSemaphoreGive(xSemaphoreInput);
            return tempInput;
        }
        else
        {
            ESP_LOGE(tag, "Variável Input ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}

int dataPID::setOutput(float output)
{
    if (xSemaphoreTake(xSemaphoreOutput, (TickType_t)10) == pdTRUE)
    {
        this->output = output;
        xSemaphoreGive(xSemaphoreOutput);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável Output ocupada, não foi possível definir valor.");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
float dataPID::getOutput()
{
    int16_t tempOutput;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphoreOutput, (TickType_t)10) == pdTRUE)
        {
            tempOutput = this->output;
            xSemaphoreGive(xSemaphoreOutput);
            return tempOutput;
        }
        else
        {
            ESP_LOGE(tag, "Variável Output ocupada, não foi possível ler valor. Tentando novamente...");
        }
    }
}

int dataPID::setSetpoint(int16_t Setpoint)
{
    if (xSemaphoreTake(xSemaphoreSetpoint, (TickType_t)10) == pdTRUE)
    {
        this->Setpoint = Setpoint;
        xSemaphoreGive(xSemaphoreSetpoint);
        return RETORNO_OK;
    }
    else
    {
        ESP_LOGE(tag, "Variável Setpoint ocupada, não foi possível gravar valor. Tentando novamente...");
        return RETORNO_VARIAVEL_OCUPADA;
    }
}
int16_t dataPID::getSetpoint()
{
    int16_t tempSetpoint;
    if (xSemaphoreTake(xSemaphoreSetpoint, (TickType_t)10) == pdTRUE)
    {
        tempSetpoint = this->Setpoint;
        xSemaphoreGive(xSemaphoreSetpoint);
    }
    else
    {
        ESP_LOGE(tag, "Não foi possível obter o setpoint do PID, retornando valor padrão.");
        return DEFAULT_SETPOINT;
    }

    return tempSetpoint;
}

int dataPID::setKp(float Kp, CarState state)
{
    switch (state)
    {
    case CAR_IN_LINE:
        if (xSemaphoreTake(xSemaphoreKp_line, (TickType_t)10) == pdTRUE)
        {
            this->Kp_line = Kp;
            xSemaphoreGive(xSemaphoreKp_line);
            return RETORNO_OK;
        }
        else
        {
            ESP_LOGE(tag, "Variável Kp_line ocupada, não foi possível definir valor.");
            return RETORNO_VARIAVEL_OCUPADA;
        }
        break;

    case CAR_IN_CURVE:
        if (xSemaphoreTake(xSemaphoreKp_curve, (TickType_t)10) == pdTRUE)
        {
            this->Kp_curve = Kp;
            xSemaphoreGive(xSemaphoreKp_curve);
            return RETORNO_OK;
        }
        else
        {
            ESP_LOGE(tag, "Variável Kp_curve ocupada, não foi possível definir valor.");
            return RETORNO_VARIAVEL_OCUPADA;
        }
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de Kp não será definido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}
float dataPID::getKp(CarState state)
{
    float tempKp;
    switch (state)
    {
    case CAR_IN_LINE:
        if (xSemaphoreTake(xSemaphoreKp_line, (TickType_t)10) == pdTRUE)
        {
            tempKp = this->Kp_line;
            xSemaphoreGive(xSemaphoreKp_line);
        }
        else
        {
            ESP_LOGE(tag, "Não foi possível obter o Kp_line do PID, retornando valor padrão.");
            return DEFAULT_KP_LINE;
        }
        break;

    case CAR_IN_CURVE:
        if (xSemaphoreTake(xSemaphoreKp_curve, (TickType_t)10) == pdTRUE)
        {
            tempKp = this->Kp_curve;
            xSemaphoreGive(xSemaphoreKp_curve);
        }
        else
        {
            ESP_LOGE(tag, "Não foi possível obter o Kp_curve do PID, retornando valor padrão.");
            return DEFAULT_KP_CURVE;
        }
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido para obter o Kp do PID, retornando valor para linha.");
        return DEFAULT_KP_LINE;
        break;
    }

    return tempKp;
}

int dataPID::setKi(float Ki, CarState state)
{
    switch (state)
    {
    case CAR_IN_LINE:
        if (xSemaphoreTake(xSemaphoreKi_line, (TickType_t)10) == pdTRUE)
        {
            this->Ki_line = Ki;
            xSemaphoreGive(xSemaphoreKi_line);
            return RETORNO_OK;
        }
        else
        {
            ESP_LOGE(tag, "Variável Ki_line ocupada, não foi possível definir valor.");
            return RETORNO_VARIAVEL_OCUPADA;
        }
        break;

    case CAR_IN_CURVE:
        if (xSemaphoreTake(xSemaphoreKi_curve, (TickType_t)10) == pdTRUE)
        {
            this->Ki_curve = Ki;
            xSemaphoreGive(xSemaphoreKi_curve);
            return RETORNO_OK;
        }
        else
        {
            ESP_LOGE(tag, "Variável Ki_curve ocupada, não foi possível definir valor.");
            return RETORNO_VARIAVEL_OCUPADA;
        }
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de Ki não será definido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}
float dataPID::getKi(CarState state)
{
    float tempKi;
    switch (state)
    {
    case CAR_IN_LINE:
        if (xSemaphoreTake(xSemaphoreKi_line, (TickType_t)10) == pdTRUE)
        {
            tempKi = this->Ki_line;
            xSemaphoreGive(xSemaphoreKi_line);
        }
        else
        {
            ESP_LOGE(tag, "Não foi possível obter o Ki_line do PID, retornando valor padrão.");
            return DEFAULT_KI_LINE;
        }
        break;

    case CAR_IN_CURVE:
        if (xSemaphoreTake(xSemaphoreKi_curve, (TickType_t)10) == pdTRUE)
        {
            tempKi = this->Ki_curve;
            xSemaphoreGive(xSemaphoreKi_curve);
        }
        else
        {
            ESP_LOGE(tag, "Não foi possível obter o Ki_curve do PID, retornando valor padrão.");
            return DEFAULT_KI_CURVE;
        }
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido para obter o Ki do PID, retornando valor para linha.");
        return DEFAULT_KI_LINE;
        break;
    }

    return tempKi;
}

int dataPID::setKd(float Kd, CarState state)
{
    switch (state)
    {
    case CAR_IN_LINE:
        if (xSemaphoreTake(xSemaphoreKd_line, (TickType_t)10) == pdTRUE)
        {
            this->Kd_line = Kd;
            xSemaphoreGive(xSemaphoreKd_line);
            return RETORNO_OK;
        }
        else
        {
            ESP_LOGE(tag, "Variável Kd_line ocupada, não foi possível definir valor.");
            return RETORNO_VARIAVEL_OCUPADA;
        }
        break;

    case CAR_IN_CURVE:
        if (xSemaphoreTake(xSemaphoreKd_curve, (TickType_t)10) == pdTRUE)
        {
            this->Kd_curve = Kd;
            xSemaphoreGive(xSemaphoreKd_curve);
            return RETORNO_OK;
        }
        else
        {
            ESP_LOGE(tag, "Variável Kd_curve ocupada, não foi possível definir valor.");
            return RETORNO_VARIAVEL_OCUPADA;
        }
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido, valor de Kd não será definido!");
        return RETORNO_ARGUMENTO_INVALIDO;
        break;
    }
}
float dataPID::getKd(CarState state)
{
    float tempKd;
    switch (state)
    {
    case CAR_IN_LINE:
        if (xSemaphoreTake(xSemaphoreKd_line, (TickType_t)10) == pdTRUE)
        {
            tempKd = this->Kd_line;
            xSemaphoreGive(xSemaphoreKd_line);
        }
        else
        {
            ESP_LOGE(tag, "Não foi possível obter o Kd_line do PID, retornando valor padrão.");
            return DEFAULT_KD_LINE;
        }
        break;

    case CAR_IN_CURVE:
        if (xSemaphoreTake(xSemaphoreKd_curve, (TickType_t)10) == pdTRUE)
        {
            tempKd = this->Kd_curve;
            xSemaphoreGive(xSemaphoreKd_curve);
        }
        else
        {
            ESP_LOGE(tag, "Não foi possível obter o Kd_curve do PID, retornando valor padrão.");
            return DEFAULT_KD_CURVE;
        }
        break;

    default:
        ESP_LOGE(tag, "Estado do Robô desconhecido para obter o Kd do PID, retornando valor para linha.");
        return DEFAULT_KD_LINE;
        break;
    }

    return tempKd;
}