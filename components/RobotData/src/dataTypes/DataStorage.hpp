#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"
#include "esp_log.h"

#include <string>
#include "DataAbstract.hpp"

#define FF_USE_LFN 1

// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

static std::string robotDataPath = "/robotdata";

static const char *TAG = "DataStorage";

// Inicialização do armazenamento FatFS
static const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = true,
    .max_files = 20,
    .allocation_unit_size = CONFIG_WL_SECTOR_SIZE};

static void mount_storage()
{
    esp_err_t err = esp_vfs_fat_spiflash_mount(robotDataPath.c_str(), "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao montar FATFS (%s)", esp_err_to_name(err));
        return;
    }
}

static FIL *get_file_pointer(std::string name)
{
    FIL *fp;
    f_open(fp, name.c_str(), FA_CREATE_NEW | FA_WRITE | FA_READ);

    if (fp == NULL)
    {
        ESP_LOGE(TAG, "Falha ao abrir arquivo %s", name.c_str());
        return NULL;
    }

    return fp;
}

static void save_data(std::string name, char *data, size_t size)
{
    FIL *fp = get_file_pointer(name);

    if (fp == NULL)
        return;

    FRESULT result = f_write(fp, data, size, NULL);
    ESP_LOGD(TAG, "Escrevendo %s, resultado: %d", name.c_str(), result);

    f_close(fp);
}

static void load_data(std::string name, char *data, size_t size)
{
    FIL *file = get_file_pointer(name);
    if (file == NULL)
        return;

    // ler dados do arquivo
    FRESULT result = f_read(file, data, size, &size);
    ESP_LOGD(TAG, "Lendo %s. Resultado: %d", name.c_str(), result);
}

#endif