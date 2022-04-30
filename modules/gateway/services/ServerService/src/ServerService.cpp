#include "ServerService.hpp"

std::list<web_socket_packet_t> ServerService::packetsReceived;
QueueHandle_t ServerService::queuePacketsReceived;

/* A simple example that demonstrates using websocket echo server
 */
static const char *TAG = "ws_echo_server";

/*
 * async send function, which we put into the httpd work queue
 */
void ServerService::ws_async_send(void *arg)
{
    static const char *data = "Async data";
    struct async_resp_arg *resp_arg = (async_resp_arg *)arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

/*
 * This handler echos back the received ws data
 * and triggers an async send if certain message received
 */
esp_err_t ServerService::ws_handler(httpd_req_t *req)
{
    uint8_t *buf = (uint8_t *)malloc(sizeof(uint8_t) * 128);
    web_socket_packet_t tempPacket;

    tempPacket.hd = req->handle;
    tempPacket.fd = httpd_req_to_sockfd(req);
    memset(&tempPacket.packet, 0, sizeof(httpd_ws_frame_t));
    tempPacket.packet.type = HTTPD_WS_TYPE_TEXT;
    tempPacket.packet.payload = buf;

    esp_err_t ret = httpd_ws_recv_frame(req, &tempPacket.packet, 128);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }

    tempPacket.packet.payload[tempPacket.packet.len] = '\0';

    ESP_LOGI(TAG, "Pacote: %s", tempPacket.packet.payload);
    ESP_LOGI(TAG, "Tipo: %d", tempPacket.packet.type);

    xQueueSend(queuePacketsReceived, &tempPacket, 0);

    // if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
    //     strcmp((char *)ws_pkt.payload, "Trigger async") == 0)
    // {
    //     return trigger_async_send(req->handle, req);
    // }

    // ret = httpd_ws_send_frame(req, &ws_pkt);

    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    // }

    return ESP_OK;
}

httpd_handle_t ServerService::start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t ws = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = ws_handler,
        .user_ctx = NULL,
        .is_websocket = true};

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

ServerService::ServerService(const char *name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    static httpd_handle_t server = NULL;

    queuePacketsReceived = xQueueCreate(10, sizeof(web_socket_packet_t));

    server = start_webserver();

    this->Start();
}

void ServerService::Run()
{
    // Loop
    for (;;)
    {
        vTaskDelay(0);
        xQueueReceive(queuePacketsReceived, &packetReceived, portMAX_DELAY);

        ESP_LOGD(GetName().c_str(), "Recebido pacote de dados");
        ESP_LOGD(GetName().c_str(), "Tamanho do pacote: %d", packetReceived.packet.len);
        ESP_LOGD(GetName().c_str(), "Dados:\n%s\n", packetReceived.packet.payload);

        std::string ret;
        
        char *line = (char *)malloc(packetReceived.packet.len + 1);
        strcpy(line, (char *)packetReceived.packet.payload);

        ESP_LOGD(GetName().c_str(), "Comando recebido via WebSocket: %s", line);

        esp_err_t err = better_console_run(line, &ret);

        ESP_LOGD(GetName().c_str(), "Retorno do comando:\n%s\nRetorno da execução: %s", ret.c_str(), esp_err_to_name(err));

        if (err == ESP_OK)
        {
            ESP_LOGD(GetName().c_str(), "Enviando pacote de dados");
            httpd_ws_frame_t ws_pkt;
            memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
            ws_pkt.payload = (uint8_t *)ret.c_str();
            ws_pkt.len = ret.length();
            ws_pkt.type = HTTPD_WS_TYPE_TEXT;

            httpd_ws_send_frame_async(packetReceived.hd, packetReceived.fd, &ws_pkt);
        }

        // this->SendAwnser(packetReceived.id, (uint8_t *)ret.c_str(), ret.size() + 1);

        free((void *)line);
        free((void *)packetReceived.packet.payload);
    }
}