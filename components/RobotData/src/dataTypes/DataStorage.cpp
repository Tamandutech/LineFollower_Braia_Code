#include "DataStorage.hpp"

std::atomic<DataStorage *> DataStorage::instance;
std::mutex DataStorage::myMutex;
std::string DataStorage::name;

DataStorage::DataStorage()
{
    s_wl_handle = WL_INVALID_HANDLE;
    basePath = "/robotdata";
    this->name = "DataStorage";

    mount_config.format_if_mount_failed = true;
    mount_config.max_files = 20;
    mount_config.allocation_unit_size = CONFIG_WL_SECTOR_SIZE;
    mounted = false;
}

bool DataStorage::is_mounted()
{
    if (!mounted)
    {
        ESP_LOGD(name.c_str(), "FATFS não montado.");
    }

    return mounted;
}

void DataStorage::mount_storage(std::string _basePath)
{
    basePath = _basePath.at(0) == '/' ? _basePath : "/" + _basePath;

    esp_err_t err = esp_vfs_fat_spiflash_mount(basePath.c_str(), "storage", &mount_config, &s_wl_handle);

    if (err != ESP_OK)
    {
        ESP_LOGE(name.c_str(), "Falha ao montar FATFS (%s)", esp_err_to_name(err));
        mounted = false;
        return;
    }
    else
    {
        ESP_LOGI(name.c_str(), "FATFS montado com sucesso em %s", basePath.c_str());
        mounted = true;
    }
}

void DataStorage::list_files()
{
    if (!is_mounted())
        return;

    DIR *d;
    struct dirent *dir;

    d = opendir(basePath.c_str());
    if (d)
    {
        ESP_LOGD(name.c_str(), "Listando diretório %s:", basePath.c_str());
        while ((dir = readdir(d)) != NULL)
        {
            ESP_LOGI(name.c_str(), "Arquivo: %s", dir->d_name);
        }
        closedir(d);
    }
}

FIL *DataStorage::get_file_pointer(std::string fileName)
{
    if (!is_mounted())
        return nullptr;

    FIL *fp = new FIL();
    f_open(fp, fileName.c_str(), FA_CREATE_NEW | FA_WRITE | FA_READ);

    if (fp == NULL)
    {
        ESP_LOGE(name.c_str(), "Falha ao abrir arquivo %s", fileName.c_str());
        return NULL;
    }

    return fp;
}

void DataStorage::save_data(std::string fileName, char *data, size_t size)
{
    if (!is_mounted())
        return;

    FIL *fp = get_file_pointer(fileName);

    if (fp == NULL)
        return;

    FRESULT result = f_write(fp, data, size, NULL);
    ESP_LOGD(name.c_str(), "Escrevendo %s, resultado: %d", fileName.c_str(), result);

    f_close(fp);
}

void DataStorage::load_data(std::string fileName, char *data, size_t size)
{
    if (!is_mounted())
        return;

    FIL *file = get_file_pointer(fileName);
    if (file == NULL)
        return;

    // ler dados do arquivo
    FRESULT result = f_read(file, data, size, &size);
    ESP_LOGD(name.c_str(), "Lendo %s. Resultado: %d", fileName.c_str(), result);
}