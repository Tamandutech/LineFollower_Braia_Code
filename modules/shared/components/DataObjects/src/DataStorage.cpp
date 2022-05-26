#include "DataStorage.hpp"

std::atomic<DataStorage *> DataStorage::instance;
std::mutex DataStorage::myMutex;
std::string DataStorage::name;

DataStorage::DataStorage()
{
    s_wl_handle = WL_INVALID_HANDLE;
    basePath = "/data";
    this->name = "DataStorage";

    mount_config.format_if_mount_failed = true;
    mount_config.max_files = 5;
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

esp_err_t DataStorage::mount_storage(std::string _basePath)
{
    if (is_mounted())
    {
        ESP_LOGE(name.c_str(), "FATFS já montado em %s. Para montar em %s primeiro desmonte o path anterior.", basePath.c_str(), _basePath.c_str());
        return ESP_FAIL;
    }

    basePath = _basePath.at(0) == '/' ? _basePath : "/" + _basePath;

    esp_err_t err = esp_vfs_fat_spiflash_mount(basePath.c_str(), "storage", &mount_config, &s_wl_handle);

    if (err != ESP_OK)
    {
        ESP_LOGE(name.c_str(), "Falha ao montar FATFS (%s)", esp_err_to_name(err));
        mounted = false;
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(name.c_str(), "FATFS montado com sucesso em %s", basePath.c_str());
        mounted = true;
        return ESP_OK;
    }
}

esp_err_t DataStorage::list_files()
{
    if (!is_mounted())
        return ESP_FAIL;

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

    return ESP_OK;
}

esp_err_t DataStorage::save_data(std::string fileName, char *data, size_t size, const char *mode)
{
    if (!is_mounted())
        return ESP_FAIL;

    FILE *f = fopen((basePath + "/" + fileName).c_str(), mode);

    if (f == NULL)
    {
        ESP_LOGE(name.c_str(), "Falha ao abrir arquivo %s para escrita", fileName.c_str());
        return ESP_FAIL;
    }

    fwrite(data, size, 1, f);
    ESP_LOGD(name.c_str(), "Escrito %s, %d bytes", fileName.c_str(), size);

    fclose(f);

    return ESP_OK;
}

esp_err_t DataStorage::load_data(std::string fileName, char *data, size_t size)
{
    if (!is_mounted())
        return ESP_FAIL;

    FILE *f = fopen((basePath + "/" + fileName).c_str(), "r");

    if (f == NULL)
    {
        ESP_LOGE(name.c_str(), "Falha ao abrir arquivo %s para leitura", fileName.c_str());
        return ESP_FAIL;
    }

    fread(data, size, 1, f);

    ESP_LOGD(name.c_str(), "Lido %s, %d bytes", fileName.c_str(), size);

    fclose(f);

    return ESP_OK;
}

esp_err_t DataStorage::load_data(std::string fileName, char **data, size_t *size)
{
    if (!is_mounted())
        return ESP_FAIL;

    FILE *f = fopen((basePath + "/" + fileName).c_str(), "rb");

    if (f == NULL)
    {
        ESP_LOGE(name.c_str(), "Falha ao abrir arquivo %s para leitura", fileName.c_str());
        return ESP_FAIL;
    }

    fseek(f, 0, SEEK_END);
    (*size) = ftell(f);

    free(*data);
    *data = (char *)malloc((*size) * sizeof(char));

    fseek(f, 0, SEEK_SET);

    fread(*data, *size, 1, f);

    ESP_LOGD(name.c_str(), "Lido %s, %d bytes", fileName.c_str(), *size);


    fclose(f);

    return ESP_OK;
}

esp_err_t DataStorage::delete_data(std::string fileName)
{
    if (!is_mounted())
        return ESP_FAIL;

    if (remove((basePath + "/" + fileName).c_str()) != 0)
    {
        ESP_LOGE(name.c_str(), "Falha ao remover arquivo %s", fileName.c_str());
        return ESP_FAIL;
    }

    ESP_LOGI(name.c_str(), "Removido %s", fileName.c_str());
    return ESP_OK;
}