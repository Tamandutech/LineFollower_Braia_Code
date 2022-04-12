
#include "esp_log.h"

extern "C"
{
  void app_main(void);
}

void app_main(void)
{
  ESP_LOGD("main", "Hello World!");
}