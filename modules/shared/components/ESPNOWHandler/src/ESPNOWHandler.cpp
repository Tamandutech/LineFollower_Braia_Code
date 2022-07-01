#include "ESPNOWHandler.h"

#define CONFIG_EXAMPLE_IPV4 true
#define CONFIG_EXAMPLE_IPV4_ONLY true

#define UDP_PORT 6534

#define MULTICAST_LOOPBACK false

#define MULTICAST_TTL 255

#define MULTICAST_IPV4_ADDR "232.10.11.12"

#define LISTEN_ALL_IF EXAMPLE_MULTICAST_LISTEN_ALL_IF

static const char *TAG = "multicast";
#ifdef CONFIG_EXAMPLE_IPV4
static const char *V4TAG = "mcast-ipv4";
#endif

std::list<PacketData> ESPNOWHandler::packetsReceived;
QueueHandle_t ESPNOWHandler::queuePacketsReceived;

ESPNOWHandler::ESPNOWHandler(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->name = name;
    ESP_LOGD(this->name.c_str(), "Criando objeto: %s", name.c_str());
    ESP_LOGD(this->name.c_str(), "Criando Semáforos");

    (xSemaphorePeerInfo) = xSemaphoreCreateMutex();

    queuePacketsReceived = xQueueCreate(10, sizeof(PacketData));

    this->ESPNOWInit(11, broadcastAddress, false);
    this->Start();
}

void ESPNOWHandler::Run()
{
    // Loop
    for (;;)
    {
        vTaskDelay(0);
        xQueueReceive(queuePacketsReceived, &packetReceived, portMAX_DELAY);

        ESP_LOGD(this->name.c_str(), "Recebido pacote de dados");
        ESP_LOGD(this->name.c_str(), "Tamanho do pacote: %d", packetReceived.size);
        ESP_LOGD(this->name.c_str(), "Dados: %s", packetReceived.data);

        if (packetReceived.type == PACKET_TYPE_CMD)
        {
            std::string ret;
            char *line = (char *)malloc(packetReceived.size);

            memcpy((void *)line, (void *)packetReceived.data, packetReceived.size);

            ESP_LOGD(this->name.c_str(), "Comando recebido via ESPNOW: %s", line);

            esp_err_t err = better_console_run(line, &ret);

            ESP_LOGD(this->name.c_str(), "Retorno do comando:\n%s\nRetorno da execução: %s\n", ret.c_str(), esp_err_to_name(err));

            this->SendAwnser(packetReceived.id, (uint8_t *)ret.c_str(), ret.size() + 1);

            free((void *)line);
        }
        else if (packetReceived.type == PACKET_TYPE_RETURN)
        {
            ESP_LOGD(this->name.c_str(), "Retorno recebido via ESPNOW, ID: %d", packetReceived.id);
            packetsReceived.push_back(packetReceived);
        }
    }
}

/* Add a socket, either IPV4-only or IPV6 dual mode, to the IPV4
   multicast group */
static int socket_add_ipv4_multicast_group(int sock, bool assign_source_if)
{
    struct ip_mreq imreq = {0};
    struct in_addr iaddr = {0};
    int err = 0;
    // Configure source interface
#if LISTEN_ALL_IF
    imreq.imr_interface.s_addr = IPADDR_ANY;
#else
    esp_netif_ip_info_t ip_info = {0};
    err = esp_netif_get_ip_info(get_example_netif(), &ip_info);
    if (err != ESP_OK)
    {
        ESP_LOGE(V4TAG, "Failed to get IP address info. Error 0x%x", err);
        goto err;
    }
    inet_addr_from_ip4addr(&iaddr, &ip_info.ip);
#endif // LISTEN_ALL_IF
    // Configure multicast address to listen to
    err = inet_aton(MULTICAST_IPV4_ADDR, &imreq.imr_multiaddr.s_addr);
    if (err != 1)
    {
        ESP_LOGE(V4TAG, "Configured IPV4 multicast address '%s' is invalid.", MULTICAST_IPV4_ADDR);
        // Errors in the return value have to be negative
        err = -1;
        goto err;
    }
    ESP_LOGI(TAG, "Configured IPV4 Multicast address %s", inet_ntoa(imreq.imr_multiaddr.s_addr));
    if (!IP_MULTICAST(ntohl(imreq.imr_multiaddr.s_addr)))
    {
        ESP_LOGW(V4TAG, "Configured IPV4 multicast address '%s' is not a valid multicast address. This will probably not work.", MULTICAST_IPV4_ADDR);
    }

    if (assign_source_if)
    {
        // Assign the IPv4 multicast source interface, via its IP
        // (only necessary if this socket is IPV4 only)
        err = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &iaddr,
                         sizeof(struct in_addr));
        if (err < 0)
        {
            ESP_LOGE(V4TAG, "Failed to set IP_MULTICAST_IF. Error %d", errno);
            goto err;
        }
    }

    err = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     &imreq, sizeof(struct ip_mreq));
    if (err < 0)
    {
        ESP_LOGE(V4TAG, "Failed to set IP_ADD_MEMBERSHIP. Error %d", errno);
        goto err;
    }

err:
    return err;
}

static int create_multicast_ipv4_socket(void)
{
    struct sockaddr_in saddr = {0};
    int sock = -1;
    int err = 0;

    uint8_t ttl = MULTICAST_TTL;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(V4TAG, "Failed to create socket. Error %d", errno);
        return -1;
    }

    // Bind the socket to any address
    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(UDP_PORT);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (err < 0)
    {
        ESP_LOGE(V4TAG, "Failed to bind socket. Error %d", errno);
        goto err;
    }

    // Assign multicast TTL (set separately from normal interface TTL)

    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(uint8_t));
    if (err < 0)
    {
        ESP_LOGE(V4TAG, "Failed to set IP_MULTICAST_TTL. Error %d", errno);
        goto err;
    }

    // this is also a listening socket, so add it to the multicast
    // group for listening...
    err = socket_add_ipv4_multicast_group(sock, true);
    if (err < 0)
    {
        goto err;
    }

    // All set, socket is configured for sending and receiving
    return sock;

err:
    close(sock);
    return -1;
}

static void mcast_example_task(void *pvParameters)
{
    while (1)
    {
        int sock;

        sock = create_multicast_ipv4_socket();
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Failed to create IPv4 multicast socket");
        }

        if (sock < 0)
        {
            // Nothing to do!
            vTaskDelay(5 / portTICK_PERIOD_MS);
            continue;
        }

        // set destination multicast addresses for sending from these sockets
        struct sockaddr_in sdestv4 = {
            .sin_family = PF_INET,
            .sin_port = htons(UDP_PORT),
        };
        // We know this inet_aton will pass because we did it above already
        inet_aton(MULTICAST_IPV4_ADDR, &sdestv4.sin_addr.s_addr);

        // Loop waiting for UDP received, and sending UDP packets if we don't
        // see any.
        int err = 1;
        while (err > 0)
        {
            struct timeval tv = {
                .tv_sec = 2,
                .tv_usec = 0,
            };
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(sock, &rfds);

            int s = select(sock + 1, &rfds, NULL, NULL, &tv);
            if (s < 0)
            {
                ESP_LOGE(TAG, "Select failed: errno %d", errno);
                err = -1;
                break;
            }
            else if (s > 0)
            {
                if (FD_ISSET(sock, &rfds))
                {
                    // Incoming datagram received
                    char recvbuf[48];
                    char raddr_name[32] = {0};

                    struct sockaddr_storage raddr; // Large enough for both IPv4 or IPv6
                    socklen_t socklen = sizeof(raddr);
                    int len = recvfrom(sock, recvbuf, sizeof(recvbuf) - 1, 0,
                                       (struct sockaddr *)&raddr, &socklen);
                    if (len < 0)
                    {
                        ESP_LOGE(TAG, "multicast recvfrom failed: errno %d", errno);
                        err = -1;
                        break;
                    }

                    // Get the sender's address as a string
                    if (raddr.ss_family == PF_INET)
                    {
                        inet_ntoa_r(((struct sockaddr_in *)&raddr)->sin_addr,
                                    raddr_name, sizeof(raddr_name) - 1);
                    }

                    ESP_LOGI(TAG, "received %d bytes from %s:", len, raddr_name);

                    recvbuf[len] = 0; // Null-terminate whatever we received and treat like a string...
                    ESP_LOGI(TAG, "%s", recvbuf);
                }
            }
            else
            { // s == 0
                // Timeout passed with no incoming data, so send something!
                static int send_count;
                const char sendfmt[] = "Multicast #%d sent by ESP32\n";
                char sendbuf[48];
                char addrbuf[32] = {0};
                int len = snprintf(sendbuf, sizeof(sendbuf), sendfmt, send_count++);
                if (len > sizeof(sendbuf))
                {
                    ESP_LOGE(TAG, "Overflowed multicast sendfmt buffer!!");
                    send_count = 0;
                    err = -1;
                    break;
                }

                struct addrinfo hints = {
                    .ai_flags = AI_PASSIVE,
                    .ai_socktype = SOCK_DGRAM,
                };
                struct addrinfo *res;
                hints.ai_family = AF_INET; // For an IPv4 socket
                int err = getaddrinfo(MULTICAST_IPV4_ADDR,
                                      NULL,
                                      &hints,
                                      &res);
                if (err < 0)
                {
                    ESP_LOGE(TAG, "getaddrinfo() failed for IPV4 destination address. error: %d", err);
                    break;
                }
                if (res == 0)
                {
                    ESP_LOGE(TAG, "getaddrinfo() did not return any addresses");
                    break;
                }
                ((struct sockaddr_in *)res->ai_addr)->sin_port = htons(UDP_PORT);
                inet_ntoa_r(((struct sockaddr_in *)res->ai_addr)->sin_addr, addrbuf, sizeof(addrbuf) - 1);
                ESP_LOGI(TAG, "Sending to IPV4 multicast address %s:%d...", addrbuf, UDP_PORT);
                err = sendto(sock, sendbuf, len, 0, res->ai_addr, res->ai_addrlen);
                freeaddrinfo(res);
                if (err < 0)
                {
                    ESP_LOGE(TAG, "IPV4 sendto failed. errno: %d", errno);
                    break;
                }
            }
        }

        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
    }
}

/// @brief Inicia o ESPNOW e registra os dados do peer
/// @param canal Canal do ESPNOW
/// @param Mac Endereço MAC do peer
/// @param criptografia Ativa ou desativa a criptografia
void ESPNOWHandler::ESPNOWInit(uint8_t canal, uint8_t *Mac, bool criptografia)
{
#ifndef ESP32_QEMU
    if (xSemaphoreTake(xSemaphorePeerInfo, (TickType_t)10) == pdTRUE)
    {
        memcpy(this->peerInfo.peer_addr, Mac, canal);
        this->peerInfo.channel = canal;
        this->peerInfo.encrypt = criptografia;
        this->peerInfo.ifidx = WIFI_IF_STA;
        xSemaphoreGive(xSemaphorePeerInfo);
    }
    else
    {
        ESP_LOGE("ESPNOWHandler", "Variável PeerInfo ocupada, não foi possível definir valor.");
    }
#endif

    WiFiHandler::getInstance()->wifi_init_sta();

#ifndef ESP32_QEMU

    if (esp_now_init() != 0)
        ESP_LOGD("ESP-NOW", "Falha ao iniciar");

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(ESPNOWHandler::OnDataSent);
    // Adiciona peer
    if (xSemaphoreTake(xSemaphorePeerInfo, (TickType_t)10) == pdTRUE)
    {
        ESP_LOGD("ESP-NOW", "PeerMac : %x|%x|%x|%x|%x|%x ", this->peerInfo.peer_addr[0], this->peerInfo.peer_addr[1], this->peerInfo.peer_addr[2], this->peerInfo.peer_addr[3], this->peerInfo.peer_addr[4], this->peerInfo.peer_addr[5]);
        if (esp_now_add_peer(&(this->peerInfo)) != ESP_OK)
        {
            ESP_LOGD("ESP-NOW", "Failed to add peer");
            xSemaphoreGive(xSemaphorePeerInfo);
            return;
        }
        else
            xSemaphoreGive(xSemaphorePeerInfo);
    }

    esp_now_register_recv_cb(ESPNOWHandler::OnDataRecv);
#else
    xTaskCreate(&mcast_example_task, "mcast_task", 4096, NULL, 5, NULL);
#endif
}

uint8_t ESPNOWHandler::SendCMD(uint8_t *data, uint16_t size)
{
    uint8_t id = GetUniqueID();
    this->Send(id, PACKET_TYPE_CMD, size, data);
    return id;
}

void ESPNOWHandler::SendAwnser(uint8_t id, uint8_t *data, uint16_t size)
{
    this->Send(id, PACKET_TYPE_RETURN, size, data);
}

void ESPNOWHandler::Send(uint8_t id, uint16_t type, uint16_t size, uint8_t *data)
{
    PacketData Packet;
    Packet.id = id;
    Packet.type = type;
    Packet.size = size;
    Packet.numToReceive = size / sizeof(Packet.data) + (size % sizeof(Packet.data) > 0);
    for (size_t i = 0, j = 1; i < size; i += sizeof(Packet.data), j++)
    {
        ESP_LOGD(this->name.c_str(), "Enviando ID: %d | pacote %d de %d", id, j, Packet.numToReceive);
        ESP_LOGD("ESP-NOW", "Mac Destino : %x|%x|%x|%x|%x|%x ", this->peerInfo.peer_addr[0], this->peerInfo.peer_addr[1], this->peerInfo.peer_addr[2], this->peerInfo.peer_addr[3], this->peerInfo.peer_addr[4], this->peerInfo.peer_addr[5]);
        Packet.numActual = j;
        memcpy(Packet.data, data + i, j == i ? size - i : sizeof(Packet.data));
#ifndef ESP32_QEMU
        esp_now_send(this->peerInfo.peer_addr, (uint8_t *)&Packet, sizeof(PacketData));
#endif

        vTaskDelay(0);
    }
}

void ESPNOWHandler::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    ESP_LOGD("ESPNOWHandler", "Mensagem recebida, bytes: %d", len);

    PacketData *Packet = (PacketData *)incomingData;

    xQueueSend(queuePacketsReceived, Packet, 0);
}

#ifndef ESP32_QEMU
void ESPNOWHandler::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    ESP_LOGD("ESP-NOW", "Mac Destino : %x|%x|%x|%x|%x|%x ", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    ESP_LOGD("ESPNOWHandler", "Status do envio: %s", status == ESP_NOW_SEND_SUCCESS ? "SUCESSO" : "FALHA");
}
#endif

PacketData ESPNOWHandler::getPacketReceived(uint8_t uniqueIdCounter, uint8_t num)
{
    // busca pacote com o id e numActual na lista de pacotes recebidos
    auto it = std::find_if(packetsReceived.begin(), packetsReceived.end(), [uniqueIdCounter, num](PacketData &p)
                           { return (p.id == uniqueIdCounter and p.numActual == num); });

    if (it != packetsReceived.end())
    {
        PacketData Packet = *it;
        packetsReceived.erase(it);
        return Packet;
    }
    else
    {
        PacketData Packet;
        Packet.id = 0;
        Packet.type = 0;
        Packet.numActual = 0;
        Packet.numToReceive = 0;
        Packet.size = 0;
        return Packet;
    }
}

uint8_t ESPNOWHandler::GetUniqueID()
{
    this->uniqueIdCounter = this->uniqueIdCounter > 254 ? 0 : this->uniqueIdCounter + 1;

    return this->uniqueIdCounter;
}