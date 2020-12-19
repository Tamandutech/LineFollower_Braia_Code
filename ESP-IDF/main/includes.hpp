#ifndef ROBOT_INCLUDES_H
#define ROBOT_INCLUDES_H

// C/C++
#include <stdbool.h>
#include <string>
#include <list>
#include <vector>

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
#include "esp_netif.h"

// Custom
#include "ESP32Encoder.h"
#include "ESP32MotorControl.h"
#include "QTRSensors.h"
#include "esp_log.h"
#include "io.h"

#endif