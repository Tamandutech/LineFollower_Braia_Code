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
        DataStorage *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
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

    void mount_storage(std::string _basePath);
    void list_files();
    FIL *get_file_pointer(std::string name);
    void save_data(std::string name, char *data, size_t size);
    void load_data(std::string name, char *data, size_t size);

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