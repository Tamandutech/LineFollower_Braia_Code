#include "Service.h"

Service::Service(std::string name, uint32_t stackDepth, UBaseType_t priority)
{
    this->name = name;
    ESP_LOGD(name.c_str(), "Criando serviço");

    if (pdPASS == xTaskCreate(task, name.c_str(), stackDepth, this, priority, &this->xTaskService))
        ESP_LOGD(name.c_str(), "Task criada no RTOS");
    else
        ESP_LOGE(name.c_str(), "Sem RAM disponivel");
        
}

void Service::task(void *_params)
{
    Service *s = static_cast<Service *>(_params);
    
    s->Setup();
    vTaskResume(NULL);
    s->Main();
}