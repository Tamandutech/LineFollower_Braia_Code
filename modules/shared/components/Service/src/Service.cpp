#include "Service.h"

Service::Service(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority)
{
    this->name = name;
    this->stackDepth = stackDepth;
    this->priority = priority;
    this->robot = robot;

    this->setupStatus = EXECSTATUS_STOPPED;
    this->mainStatus = EXECSTATUS_STOPPED;

    ESP_LOGD(name, "(%p): Criando serviço", this);
}

void Service::task(void *_params)
{
    Service *s = static_cast<Service *>(_params);

    if (s->setupStatus == EXECSTATUS_STOPPED)
    {
        s->setupStatus = EXECSTATUS_RUNNING;
        ESP_LOGD(s->name, "(%p): Setup (Async): Iniciando.", s);
        s->Setup();
        s->setupStatus = EXECSTATUS_FINISHED;
        ESP_LOGD(s->name, "(%p): Setup (Async): Concluído.", s);

        vTaskSuspend(NULL);
    }

    if (s->mainStatus == EXECSTATUS_STOPPED)
    {
        s->mainStatus = EXECSTATUS_RUNNING;
        ESP_LOGD(s->name, "(%p): Main (Async): Iniciando.", s);
        s->Main();
        s->mainStatus = EXECSTATUS_FINISHED;
        ESP_LOGD(s->name, "(%p): Main (Async): Concluído.", s);
    }
}

void Service::createAsync()
{
    if (pdPASS == xTaskCreate(task, name, stackDepth, this, priority, &this->xTaskService))
        ESP_LOGD(name, "(%p): Task criada no RTOS\n - pvParameters: %p", this, this);
    else
        ESP_LOGE(name, "(%p): Sem RAM disponivel", this);
}

void Service::create()
{
    setupStatus = EXECSTATUS_RUNNING;
    ESP_LOGD(name, "(%p): Setup: Iniciando.", this);
    Setup();
    setupStatus = EXECSTATUS_FINISHED;
    ESP_LOGD(name, "(%p): Setup: Concluído.", this);
}

void Service::startAsync()
{
    if (xTaskService != NULL)
        vTaskResume(this->xTaskService);
    else
        this->createAsync();
}

void Service::start()
{
    setupStatus = EXECSTATUS_RUNNING;
    ESP_LOGD(name, "(%p): Main: Iniciando.", this);
    Main();
    setupStatus = EXECSTATUS_FINISHED;
    ESP_LOGD(name, "(%p): Main: Concluído.", this);
}
