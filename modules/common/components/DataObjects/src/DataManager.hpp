#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <atomic>
#include <iostream>
#include <string>
#include <mutex>
#include <vector>

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

    void loadAllParamData();
    void loadAllRuntimeData();

    uint8_t getRegistredParamDataCount();
    std::string listRegistredParamData();

private:
    static std::atomic<DataManager *> instance;
    static std::mutex instanceMutex;

    static std::vector<IDataAbstract *> dataParamList;
    static std::mutex dataParamListMutex;


    static std::vector<IDataAbstract *> dataRuntimeList;
    static std::mutex dataRuntimeListMutex;

    std::vector<IDataAbstract *> dataParamChangedList;
    std::mutex dataParamChangedListMutex;

    static std::string name;

    void registerData(IDataAbstract *data, std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex);
    void saveAllData(std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex);
    void loadAllData(std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex);

    DataManager();
    bool is_mounted();
};

#endif