#pragma once

// C/C++
#include <stdbool.h>
#include <string>
#include <list>

// Espressif (ESP-IDF)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "tcpip_adapter.h"

// Custom
#include "ESP32Encoder.h"
#include "ESP32MotorControl.h"
#include "QTRSensors.h"
#include "esp_log.h"
#include "io.h"