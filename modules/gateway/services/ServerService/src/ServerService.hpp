#ifndef WEB_SOCKET_SERVICE_HPP
#define WEB_SOCKET_SERVICE_HPP

#include "thread.hpp"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "cJSON.h"

#include <esp_http_server.h>

#include "better_console.hpp"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

struct async_resp_arg
{
    httpd_handle_t hd;
    int fd;
};

struct web_socket_packet_t{
    httpd_handle_t hd;
    int fd;
    httpd_ws_frame_t packet;
};

class ServerService : public Thread
{
public:
    ServerService(const char *name, uint32_t stackDepth, UBaseType_t priority);
    void Run() override;

    /*
     * async send function, which we put into the httpd work queue
     */
    static void ws_async_send(void *arg);

    static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req);

private:
    /*
     * This handler echos back the received ws data
     * and triggers an async send if certain message received
     */
    static esp_err_t ws_handler(httpd_req_t *req);

    static httpd_handle_t start_webserver(void);

    web_socket_packet_t packetReceived;
    static QueueHandle_t queuePacketsReceived;

    static std::list<web_socket_packet_t> packetsReceived;
    static SemaphoreHandle_t xSemaphorePacketsReceived;
};

#endif