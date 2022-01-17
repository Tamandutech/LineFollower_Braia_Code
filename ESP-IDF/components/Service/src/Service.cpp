#include "Service.h"

Service::Service(const char* name, uint32_t stackDepth, UBaseType_t priority)
{
    this->name = name;
    this->stackDepth = stackDepth;
    this->priority = priority;

    ESP_LOGD(name, "Criando serviço");
}

void Service::task(void *_params)
{
    Service *s = static_cast<Service *>(_params);

    s->Setup();
    // vTaskSuspend(NULL);
    s->Main();
}

void Service::create()
{
    if (pdPASS == xTaskCreate(task, name, stackDepth, this, priority, &this->xTaskService))
        ESP_LOGD(name, "Task criada no RTOS");
    else
        ESP_LOGE(name, "Sem RAM disponivel");
}