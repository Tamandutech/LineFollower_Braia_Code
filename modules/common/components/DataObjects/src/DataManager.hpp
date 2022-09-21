#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <atomic>
#include <iostream>
#include <string>
#include <mutex>
#include <vector>

#include "cJSON.h"

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include "IDataAbstract.hpp"

#include "esp_log.h"

class DataManager
{

public:
    static DataManager *getInstance()
    {
        DataManager *sin = instance.load(std::memory_order_acquire);
        if (!sin)
        {
            std::lock_guard<std::mutex> myLock(instanceMutex);
            sin = instance.load(std::memory_order_relaxed);
            if (!sin)
            {
                sin = new DataManager();
                instance.store(sin, std::memory_order_release);
            }
        }

        return sin;
    };

    void registerParamData(IDataAbstract *data);
    void registerParamDataChanged(std::string name);
    void registerRuntimeData(IDataAbstract *data);

    void saveAllParamData();
    void saveAllParamDataChanged();
    void saveAllRuntimeData();

    void setParam(std::string name, std::string value, bool savedata = true);
    std::string getParam(std::string name, std::string ctrl);

    void setRuntime(std::string name, std::string value);
    std::string getRuntime(std::string name);

    void loadAllParamData();
    void loadAllRuntimeData();

    uint8_t getRegistredParamDataCount();
    std::string listRegistredParamData();

    std::string listRegistredRuntimeData();

    // name: nome do dado <ObjetoPai>.<Objeto>
    // time_in_ms: tempo em ms do intervalo de envio
    void setStreamInterval(std::string name, uint32_t time_in_ms);

    // Quantidade de dados prontos para o stream
    int NumItemsReadyStream();

    // Retorna todos os dados que devem fazer stream naquele momento.
    cJSON* getStreamData();

private:
    static std::atomic<DataManager *> instance;
    static std::mutex instanceMutex;

    static std::vector<IDataAbstract *> dataParamList;
    static std::mutex dataParamListMutex;


    static std::vector<IDataAbstract *> dataRuntimeList;
    static std::mutex dataRuntimeListMutex;

    static std::vector<IDataAbstract *> dataStreamList; // dados que devem ser transmitidos para a dashboard
    static std::mutex dataStreamListMutex;

    std::vector<IDataAbstract *> dataParamChangedList;
    std::mutex dataParamChangedListMutex;

    static std::string name;

    void registerData(IDataAbstract *data, std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex);
    void saveAllData(std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex);
    void loadAllData(std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex);

    //Obtém o ponteiro (iterator) para o dado em uma determinada lista
    std::vector<IDataAbstract *>::iterator getDataListIterator(IDataAbstract *data, std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex);
    DataManager();
    bool is_mounted();
};

#endif