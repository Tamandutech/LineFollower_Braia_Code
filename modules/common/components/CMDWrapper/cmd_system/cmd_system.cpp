/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <sstream>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "esp_log.h"
#include "better_console.hpp"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_sleep.h"
#include "spi_flash_mmap.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cmd_system.hpp"
#include "sdkconfig.h"

#include "DataStorage.hpp"
#include "RobotData.h"
#include "SensorsService.hpp"
#include "CarStatusService.hpp"

#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
#define WITH_TASKS_INFO 1
#endif

static const char *TAG = "CMD_SYSTEM";
CarState lastState = CAR_STOPPED;

adc_oneshot_unit_handle_t ADChandle;
adc_cali_handle_t adc1_cali_chan0_handle;

static void register_free(void);
static void register_heap(void);
static void register_version(void);
static void register_restart(void);
static void register_bat_voltage(void);
static void register_start(void);
static void register_resume(void);
static void register_pause(void);
static void register_calibrate(void);
#if WITH_TASKS_INFO
static void register_tasks(void);
#endif
static void register_delete_data(void);

void register_system_common(void)
{
    register_free();
    register_heap();
    register_version();
    register_restart();
    register_start();
    register_resume();
    register_pause();
    register_calibrate();
    register_bat_voltage();
#if WITH_TASKS_INFO
    register_tasks();
#endif
    register_delete_data();
}

void register_system(adc_oneshot_unit_handle_t adc_handle)
{
    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t ADCconfig = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ADChandle = adc_handle;
    ESP_ERROR_CHECK(adc_oneshot_config_channel(ADChandle, ADC_CHANNEL_0, &ADCconfig));

    //-------------ADC1 Calibration---------------//
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &adc1_cali_chan0_handle));
    register_system_common();
}

/* 'version' command */
static std::string get_version(int argc, char **argv)
{
    esp_chip_info_t info;
    esp_chip_info(&info);
    uint32_t size_flash_chip;
    esp_flash_get_size(NULL, &size_flash_chip);


    std::stringstream ss;
    ss << "IDF Version: " << esp_get_idf_version() << std::endl;
    ss << "Chip info:" << std::endl;
    ss << "    Model: " << info.model << std::endl;
    ss << "    Cores: " << info.cores << std::endl;
    ss << "    Revision: " << info.revision << std::endl;
    ss << "    Features: ";
    ss << (info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "");
    ss << (info.features & CHIP_FEATURE_BLE ? "/BLE" : "");
    ss << (info.features & CHIP_FEATURE_BT ? "/BT" : "");
    ss << (info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:");
    ss << (size_flash_chip / (1024 * 1024));
    ss << "MB";
    ss << std::endl;
    ss << "    Revision: " << info.revision << std::endl;

    return ss.str();
}

static void register_version(void)
{
    const better_console_cmd_t cmd = {
        .command = "version",
        .help = "Get version of chip and SDK",
        .hint = NULL,
        .func = &get_version,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&cmd));
}

static std::string bat_voltage(int argc, char **argv)
{
    int calVoltage = 0;
    int adc_raw;

    esp_err_t err;
    do {
        err = adc_oneshot_read(ADChandle, ADC_CHANNEL_0, &adc_raw);
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
    } while (err == ESP_ERR_TIMEOUT);
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw, &calVoltage));
    
    calVoltage *= 3.7;
    return (std::to_string(calVoltage) + "mV");
}

static void register_bat_voltage(void)
{
    const better_console_cmd_t cmd = {
        .command = "bat_voltage",
        .help = "Obtém a tensão da bateria",
        .hint = NULL,
        .func = &bat_voltage,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&cmd));
}

static std::string start(int argc, char **argv)
{
    auto status = Robot::getInstance()->getStatus();
    auto MappingData = Robot::getInstance()->getMappingData();
    SensorsService::getInstance()->Suspend();
    SensorsService::getInstance()->calibAllsensors();
    SensorsService::getInstance()->Resume();
    if (MappingData->TrackSideMarks->getSize() <= 0)
    {
        status->robotState->setData(CAR_MAPPING);
    }
    else
    {
        status->robotState->setData(CAR_ENC_READING);
    }
    xSemaphoreGive(CarStatusService::getInstance()->SemaphoreButton);
    return ("O robô começará a se mover");
}

static void register_start(void)
{
    const better_console_cmd_t cmd = {
        .command = "start",
        .help = "Calibra e faz o robô se mover",
        .hint = NULL,
        .func = &start,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&cmd));
}

static std::string resume(int argc, char **argv)
{
    auto status = Robot::getInstance()->getStatus();
    auto MappingData = Robot::getInstance()->getMappingData();
    if(!status->TunningMode->getData())
    {
        if (MappingData->TrackSideMarks->getSize() <= 0)
        {
            status->robotState->setData(CAR_MAPPING);
        }
        else
        {
            status->robotState->setData(CAR_ENC_READING);
        }
    }
    status->robotState->setData(lastState);
    xSemaphoreGive(CarStatusService::getInstance()->SemaphoreButton);
    status->robotPaused->setData(false);

    return ("O robô voltará a andar");
}

static void register_resume(void)
{
    const better_console_cmd_t cmd = {
        .command = "resume",
        .help = "Faz o robô voltar a se mover",
        .hint = NULL,
        .func = &resume,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&cmd));
}

static std::string pause(int argc, char **argv)
{
    auto status = Robot::getInstance()->getStatus();
    if(status->robotState->getData() != CAR_STOPPED)
    {
        lastState = (CarState) status->robotState->getData();
        status->robotPaused->setData(true);
        status->robotState->setData(CAR_STOPPED);
        vTaskDelay(0);
        DataManager::getInstance()->saveAllParamDataChanged();
        LEDsService::getInstance()->LedComandSend(LED_POSITION_FRONT, LED_COLOR_BLACK, 1);
    }
    return ("O robô será pausado");
}

static void register_pause(void)
{
    const better_console_cmd_t cmd = {
        .command = "pause",
        .help = "Faz o robô parar de se mover",
        .hint = NULL,
        .func = &pause,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&cmd));
}

static std::string calibrate(int argc, char **argv)
{
    SensorsService::getInstance()->Suspend();
    SensorsService::getInstance()->calibAllsensors();
    SensorsService::getInstance()->Resume();
    return ("O robô será calibrado");
}

static void register_calibrate(void)
{
    const better_console_cmd_t cmd = {
        .command = "calibrate",
        .help = "Faz o robô começar a calibração e pausa ele",
        .hint = NULL,
        .func = &calibrate,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&cmd));
}

/** 'restart' command restarts the program */

static std::string restart(int argc, char **argv)
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();
    return "OK";
}

static void register_restart(void)
{
    const better_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&cmd));
}

/** 'free' command prints available heap memory */

static std::string free_mem(int argc, char **argv)
{
    return std::to_string(esp_get_free_heap_size());
}

static void register_free(void)
{
    const better_console_cmd_t cmd = {
        .command = "free",
        .help = "Get the current size of free heap memory",
        .hint = NULL,
        .func = &free_mem,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&cmd));
}

/* 'heap' command prints minumum heap size */
static std::string heap_size(int argc, char **argv)
{
    return std::to_string(heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
}

static void register_heap(void)
{
    const better_console_cmd_t heap_cmd = {
        .command = "heap",
        .help = "Get minimum size of free heap memory that was available during program execution",
        .hint = NULL,
        .func = &heap_size,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&heap_cmd));
}

/** 'tasks' command prints the list of tasks and related information */
#if WITH_TASKS_INFO

static std::string tasks_info(int argc, char **argv)
{
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = (char *)malloc(uxTaskGetNumberOfTasks() * bytes_per_task);

    std::stringstream ss;

    if (task_list_buffer == NULL)
    {
        return "Falha ao alocar memória para vTaskList";
    }

    // fputs("Nome da Task\tStatus\tPrio\tStack\tTask\tCore", stdout);
    ss << "Nome da Task\tStatus\tPrio\tStack\tTask\tCore";
    // fputs("\n", stdout);
    ss << "\n";

    vTaskList(task_list_buffer);
    // fputs(task_list_buffer, stdout);
    ss << task_list_buffer;
    // fputs("\n", stdout);

    free(task_list_buffer);
    return ss.str();
}

static void register_tasks(void)
{
    const better_console_cmd_t cmd = {
        .command = "tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &tasks_info,
        .argtable = NULL};
    ESP_ERROR_CHECK(better_console_cmd_register(&cmd));
}

#endif // WITH_TASKS_INFO

static struct
{
    struct arg_str *filename;
    struct arg_end *end;
} delete_data_args;

static std::string delete_data(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&delete_data_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, delete_data_args.end, argv[0]);
        return "NOK";
    }

    ESP_LOGI(TAG, "Excluindo arquivo: %s", delete_data_args.filename->sval[0]);

    if (ESP_OK == DataStorage::getInstance()->delete_data(delete_data_args.filename->sval[0]))
        return "OK";
    else
        return "NOK";
}

void register_delete_data(void)
{
    delete_data_args.filename = arg_str1(NULL, NULL, "<arquivo>", "Nome do arquivo a ser excluído.");
    delete_data_args.end = arg_end(2);

    const better_console_cmd_t delete_data_cmd = {
        .command = "delete_data",
        .help = "Apaga um arquivo da memória flash.",
        .hint = NULL,
        .func = &delete_data,
        .argtable = &delete_data_args};

    better_console_cmd_register(&delete_data_cmd);
}