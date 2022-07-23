#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <atomic>
#include <iostream>
#include <string>
#include <mutex>

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include "esp_log.h"

class DataStorage
{

public:
    static DataStorage *getInstance()
    {
        ESP_LOGD("DataStorage", "Adquirindo instância...");

        DataStorage *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
            ESP_LOGD("DataStorage", "Instância não existe, criando...");

            std::lock_guard<std::mutex> myLock(myMutex);
            sin = instance.load(std::memory_order_relaxed);
            if (!sin)
            {
                sin = new DataStorage();
                instance.store(sin, std::memory_order_release);
            }
        }

        return sin;
    };

    esp_err_t mount_storage(std::string _basePath);
    esp_err_t list_files();

    esp_err_t save_data(std::string name, char *data, size_t size, const char *mode = "wb");

    esp_err_t load_data(std::string name, char *data, size_t size);
    esp_err_t load_data(std::string name, char **data, size_t *size);

    esp_err_t delete_data(std::string name);

private:
    static std::atomic<DataStorage *> instance;
    static std::mutex myMutex;
    static std::string name;

    wl_handle_t s_wl_handle;
    std::string basePath;
    esp_vfs_fat_sdmmc_mount_config_t mount_config;
    bool mounted;

    DataStorage();
    bool is_mounted();
};

#endif