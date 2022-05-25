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

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
const char *ServerService::get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest)
    {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash)
    {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize)
    {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

esp_err_t ServerService::set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf"))
    {
        return httpd_resp_set_type(req, "application/pdf");
    }
    else if (IS_FILE_EXT(filename, ".html"))
    {
        return httpd_resp_set_type(req, "text/html");
    }
    else if (IS_FILE_EXT(filename, ".html.gz"))
    {
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        return httpd_resp_set_type(req, "text/html");
    }
    else if (IS_FILE_EXT(filename, ".jpeg"))
    {
        return httpd_resp_set_type(req, "image/jpeg");
    }
    else if (IS_FILE_EXT(filename, ".ico"))
    {
        return httpd_resp_set_type(req, "image/x-icon");
    }
    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}

esp_err_t ServerService::index_html_get_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, NULL, 0); // Response body can be empty
    return ESP_OK;
}

/* Handler to download a file kept on the server */
esp_err_t ServerService::download_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;

    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri, sizeof(filepath));
    if (!filename)
    {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    if (stat(filepath, &file_stat) == -1)
    {
        /* If file not present on SPIFFS check if URI
         * corresponds to one of the hardcoded paths */
        if (filename[strlen(filename) - 1] == '/' || strcmp(filename, "/index.html") == 0)
        {
            return index_html_get_handler(req);
        }
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    fd = fopen(filepath, "r");
    if (!fd)
    {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
    set_content_type_from_file(req, filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
    size_t chunksize;
    do
    {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunksize > 0)
        {
            /* Send the buffer contents as HTTP response chunk */
            ESP_LOGD(TAG, "Enviando %d bytes", chunksize);
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK)
            {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t ServerService::upload_post_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;

    /* Skip leading "/upload" from URI to get filename */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri + sizeof("/upload") - 1, sizeof(filepath));
    if (!filename)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/')
    {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        return ESP_FAIL;
    }

    if (stat(filepath, &file_stat) == 0)
    {
        ESP_LOGE(TAG, "File already exists : %s", filepath);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
        return ESP_FAIL;
    }

    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE)
    {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "File size must be less than " MAX_FILE_SIZE_STR "!");
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
    }

    fd = fopen(filepath, "w");
    if (!fd)
    {
        ESP_LOGE(TAG, "Failed to create file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Receiving file : %s...", filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *buf = ((struct file_server_data *)req->user_ctx)->scratch;
    int received;

    /* Content length of the request gives
     * the size of the file being uploaded */
    int remaining = req->content_len;

    while (remaining > 0)
    {

        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        if ((received = httpd_req_recv(req, buf, MIN(remaining, SCRATCH_BUFSIZE))) <= 0)
        {
            if (received == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry if timeout occurred */
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File reception failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        /* Write buffer content to file on storage */
        if (received && (received != fwrite(buf, 1, received, fd)))
        {
            /* Couldn't write everything to file!
             * Storage may be full? */
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File write failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
            return ESP_FAIL;
        }

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
    }

    /* Close file upon upload completion */
    fclose(fd);
    ESP_LOGI(TAG, "File reception complete");

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}

esp_err_t ServerService::delete_post_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    struct stat file_stat;

    /* Skip leading "/delete" from URI to get filename */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri + sizeof("/delete") - 1, sizeof(filepath));
    if (!filename)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/')
    {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        return ESP_FAIL;
    }

    if (stat(filepath, &file_stat) == -1)
    {
        ESP_LOGE(TAG, "File does not exist : %s", filename);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File does not exist");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Deleting file : %s", filename);
    /* Delete file */
    unlink(filepath);

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_sendstr(req, "File deleted successfully");
    return ESP_OK;
}

httpd_handle_t ServerService::start_webserver(std::string base_path)
{
    static struct file_server_data *server_data = NULL;

    if (server_data)
    {
        ESP_LOGE(TAG, "File server already started");
    }

    /* Allocate memory for server data */
    server_data = (file_server_data *)calloc(1, sizeof(struct file_server_data));
    if (!server_data)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
    }
    strlcpy(server_data->base_path, base_path.c_str(),
            sizeof(server_data->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_uri_t ws = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = ws_handler,
        .user_ctx = NULL,
        .is_websocket = true};

    /* URI handler for getting uploaded files */
    httpd_uri_t file_download = {
        .uri = "/*", // Match all URIs of type /path/to/file
        .method = HTTP_GET,
        .handler = download_get_handler,
        .user_ctx = server_data // Pass server data as context
    };

    /* URI handler for uploading files to server */
    httpd_uri_t file_upload = {
        .uri = "/upload/*", // Match all URIs of type /upload/path/to/file
        .method = HTTP_POST,
        .handler = upload_post_handler,
        .user_ctx = server_data // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_upload);

    /* URI handler for deleting files from server */
    httpd_uri_t file_delete = {
        .uri = "/delete/*", // Match all URIs of type /delete/path/to/file
        .method = HTTP_POST,
        .handler = delete_post_handler,
        .user_ctx = server_data // Pass server data as context
    };

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");

        httpd_register_uri_handler(server, &ws);
        httpd_register_uri_handler(server, &file_download);
        httpd_register_uri_handler(server, &file_upload);
        httpd_register_uri_handler(server, &file_delete);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

ServerService::ServerService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    static httpd_handle_t server = NULL;

    queuePacketsReceived = xQueueCreate(10, sizeof(web_socket_packet_t));

    storage = storage->getInstance();
    storage->mount_storage("/data");

    server = start_webserver("/data");

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

        std::string wsMsg;

        char *line = (char *)malloc(packetReceived.packet.len + 1);
        strcpy(line, (char *)packetReceived.packet.payload);

        ESP_LOGD(GetName().c_str(), "Comando recebido via WebSocket: %s", line);

        esp_err_t err = better_console_run(line, &ret);

        ESP_LOGD(GetName().c_str(), "Retorno do comando:\n%s\nRetorno da execução: %s\n", ret.c_str(), esp_err_to_name(err));

        if (err == ESP_OK)
        {
            cJSON *root;
            root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "cmdExecd", line);
            cJSON_AddStringToObject(root, "data", ret.c_str());

            char *my_json_string = cJSON_Print(root);

            ESP_LOGD(GetName().c_str(), "Enviando pacote de dados, tamanho: %d", strlen(my_json_string));

            httpd_ws_frame_t ws_pkt;
            memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
            ws_pkt.payload = (uint8_t *)my_json_string;
            ws_pkt.len = strlen(my_json_string);
            ws_pkt.type = HTTPD_WS_TYPE_TEXT;

            httpd_ws_send_frame_async(packetReceived.hd, packetReceived.fd, &ws_pkt);

            cJSON_Delete(root);
            cJSON_free(my_json_string);
        }

        // this->SendAwnser(packetReceived.id, (uint8_t *)ret.c_str(), ret.size() + 1);

        free((void *)line);
        free((void *)packetReceived.packet.payload);
    }
}