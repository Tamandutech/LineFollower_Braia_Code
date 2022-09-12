#include <string>

#include "cmd_wifi.hpp"

#include "esp_wifi.h"

#include "esp_log.h"

#include "better_console.hpp"
#include "argtable3/argtable3.h"

void register_cmd_wifi(void)
{
    register_channel_get();
}

static std::string channel_get(int argc, char **argv)
{
    uint8_t pChannel;
    wifi_second_chan_t sChannel;
    esp_wifi_get_channel(&pChannel, &sChannel);

    return ("pChannel:" + std::to_string(pChannel) + " sChannel:" + std::to_string(sChannel));
}

void register_channel_get(void)
{
    const better_console_cmd_t param_set_cmd = {
        .command = "wifi_channel_get",
        .help = "Retorna o canal atual do rádio WiFi.",
        .hint = NULL,
        .func = &channel_get,
        .argtable = NULL};

    better_console_cmd_register(&param_set_cmd);
}