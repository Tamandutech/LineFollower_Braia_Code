#ifndef WEB_SOCKET_SERVICE_HPP
#define WEB_SOCKET_SERVICE_HPP

#include "thread.hpp"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "cJSON.h"

#include "esp_vfs.h"
#include "esp_vfs_fat.h"

#include <esp_http_server.h>

#include "better_console.hpp"

#include "DataStorage.hpp"

using namespace cpp_freertos;

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

/* Max length a file path can have on storage */
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

/* Max size of an individual file. Make sure this
 * value is same as that set in upload_script.html */
#define MAX_FILE_SIZE (600 * 1024) // 200 KB
#define MAX_FILE_SIZE_STR "200KB"

/* Scratch buffer size */
#define SCRATCH_BUFSIZE 8192

struct file_server_data
{
    /* Base path of file storage */
    char base_path[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
};

struct async_resp_arg
{
    httpd_handle_t hd;
    int fd;
};

struct web_socket_packet_t
{
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
    DataStorage *storage;

    static const char *get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize);
    static esp_err_t index_html_get_handler(httpd_req_t *req);
    static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);

    /*
     * This handler echos back the received ws data
     * and triggers an async send if certain message received
     */
    static esp_err_t ws_handler(httpd_req_t *req);
    static esp_err_t download_get_handler(httpd_req_t *req);
    static esp_err_t upload_post_handler(httpd_req_t *req);
    static esp_err_t delete_post_handler(httpd_req_t *req);

    static httpd_handle_t start_webserver(std::string base_path);

    web_socket_packet_t packetReceived;
    static QueueHandle_t queuePacketsReceived;

    static std::list<web_socket_packet_t> packetsReceived;
    static SemaphoreHandle_t xSemaphorePacketsReceived;
};

#endif