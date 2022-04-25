#ifndef WEB_SOCKET_SERVICE_HPP
#define WEB_SOCKET_SERVICE_HPP

#include "thread.hpp"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"

#include <esp_http_server.h>

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

class ServerService : public Thread
{
public:
    ServerService(const char *name, uint32_t stackDepth, UBaseType_t priority);
    void Run() override;

private:

};

#endif