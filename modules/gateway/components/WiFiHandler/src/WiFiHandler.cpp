#include "WiFiHandler.h"

EventGroupHandle_t WiFiHandler::s_wifi_event_group;

/* The examples use WiFi configuration that you can set via project configuration menu.
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_CHANNEL 11
#define EXAMPLE_MAX_STA_CONN 5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

int WiFiHandler::s_retry_num = 0;

void WiFiHandler::wifi_event_handler(void *arg, esp_event_base_t event_base,
                                     int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(WiFiHandler::getInstance()->GetName().c_str(), "Dispositivo " MACSTR " conectou, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(WiFiHandler::getInstance()->GetName().c_str(), "Dispositivo " MACSTR " desconectou, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void WiFiHandler::event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 10)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(WiFiHandler::getInstance()->GetName().c_str(), "Tentando conectar novamente ao AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(WiFiHandler::getInstance()->GetName().c_str(), "Falha ao conectar ao AP");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WiFiHandler::getInstance()->GetName().c_str(), "Conectou com IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void WiFiHandler::wifi_init_sta(void)
{
    if (!wifiAlreadyInit)
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
#ifndef ESP32_QEMU
        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(esp_wifi_set_channel(EXAMPLE_ESP_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE));
#endif
        wifiAlreadyInit = true;
    }
    else
    {
        ESP_LOGI(GetName().c_str(), "WiFi já inicializado.");
    }
}

void WiFiHandler::wifi_init_sta(std::string ssid, std::string password)
{
    if (!wifiAlreadyInit)
    {
        s_wifi_event_group = xEventGroupCreate();

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

        esp_netif_create_default_wifi_sta();

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &(WiFiHandler::event_handler),
                                                            NULL,
                                                            &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &(WiFiHandler::event_handler),
                                                            NULL,
                                                            &instance_got_ip));

        wifi_config_t wifi_config = {.sta{.threshold{.authmode = WIFI_AUTH_WPA2_PSK}}};
        strcpy((char *)wifi_config.sta.ssid, ssid.c_str());
        strcpy((char *)wifi_config.sta.password, password.c_str());

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
#ifndef ESP32_QEMU
        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
        ESP_ERROR_CHECK(esp_wifi_start());
#endif

        ESP_LOGI(GetName().c_str(), "Estação inicializada.");

        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
         * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                               WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);

        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
         * happened. */
        if (bits & WIFI_CONNECTED_BIT)
        {
            ESP_LOGI(GetName().c_str(), "Conectado ao AP SSID:%s Senha:%s",
                     ssid.c_str(), password.c_str());
        }
        else if (bits & WIFI_FAIL_BIT)
        {
            ESP_LOGI(GetName().c_str(), "Falha ao conectar no AP SSID:%s, Senha:%s",
                     ssid.c_str(), password.c_str());
        }
        else
        {
            ESP_LOGE(GetName().c_str(), "EVENTO INESPERADO");
        }

        /* The event will not be processed after unregister */
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
        vEventGroupDelete(s_wifi_event_group);

        wifiAlreadyInit = true;
    }
    else
    {
        ESP_LOGI(GetName().c_str(), "WiFi já inicializado.");
    }
}

void WiFiHandler::wifi_init_softap(std::string ssid, std::string password)
{
    if (!wifiAlreadyInit)
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

        esp_netif_create_default_wifi_ap();

        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            NULL));

        wifi_config_t wifi_config = {
            .ap = {
                .channel = EXAMPLE_ESP_WIFI_CHANNEL,
                .authmode = WIFI_AUTH_WPA_WPA2_PSK,
                .max_connection = EXAMPLE_MAX_STA_CONN}};

        strcpy((char *)wifi_config.ap.ssid, ssid.c_str());
        wifi_config.ap.ssid_len = strlen((char *)wifi_config.ap.ssid);
        strcpy((char *)wifi_config.ap.password, password.c_str());

        if (strlen(password.c_str()) == 0)
        {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }

        esp_wifi_set_mode(WIFI_MODE_APSTA);
        esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
        esp_wifi_start();

        ESP_LOGI(GetName().c_str(), "AP criada. SSID:%s Senha:%s Canal:%d",
                 ssid.c_str(), password.c_str(), EXAMPLE_ESP_WIFI_CHANNEL);

        wifiAlreadyInit = true;
    }
    else
    {
        ESP_LOGI(GetName().c_str(), "WiFi já inicializado.");
    }
}

WiFiHandler::WiFiHandler(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void WiFiHandler::Run()
{
}
