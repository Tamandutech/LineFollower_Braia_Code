/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#include <string>

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

// Register all system functions
void register_system(adc_oneshot_unit_handle_t adc_handle);

// Register common system functions: "version", "restart", "free", "heap", "tasks", "bat_voltage"
void register_system_common(void);

// Register deep and light sleep functions
void register_system_sleep(void);
