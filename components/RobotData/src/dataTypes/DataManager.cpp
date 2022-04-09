#include "DataManager.hpp"

std::atomic<DataManager *> DataManager::instance;
std::mutex DataManager::instanceMutex;
std::string DataManager::name;

std::vector<IDataAbstract *> DataManager::dataParamList;
std::mutex DataManager::dataParamListMutex;

std::vector<IDataAbstract *> DataManager::dataRuntimeList;
std::mutex DataManager::dataRuntimeListMutex;

DataManager::DataManager()
{
    this->name = "DataManager";
}

void DataManager::registerData(IDataAbstract *data, std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex)
{
    std::lock_guard<std::mutex> myLock(*dataListMutex);
    dataList->push_back(data);
}

void DataManager::registerParamData(IDataAbstract *data)
{
    ESP_LOGD(name.c_str(), "Registrando dado parametrizado: %s (%p)", data->getName().c_str(), data);
    registerData(data, &dataParamList, &dataParamListMutex);
}

void DataManager::registerRuntimeData(IDataAbstract *data)
{
    ESP_LOGD(name.c_str(), "Registrando dado runtime: %s (%p)", data->getName().c_str(), data);
    registerData(data, &dataRuntimeList, &dataRuntimeListMutex);
}

void DataManager::saveAllData(std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex)
{
    std::lock_guard<std::mutex> myLock(*dataListMutex);
    for (auto data : *dataList)
    {
        data->saveData();
    }
}

void DataManager::saveAllParamData()
{
    ESP_LOGD(name.c_str(), "Salvando dados parametrizados, %d registrados...", dataParamList.size());
    saveAllData(&dataParamList, &dataParamListMutex);
}

void DataManager::saveAllRuntimeData()
{
    ESP_LOGD(name.c_str(), "Salvando dados runtime, %d registrados...", dataRuntimeList.size());
    saveAllData(&dataRuntimeList, &dataRuntimeListMutex);
}

void DataManager::loadAllData(std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex)
{
    ESP_LOGD(name.c_str(), "Carregando dados, %d registrados...", dataList->size());

    std::lock_guard<std::mutex> myLock(*dataListMutex);
    for (auto data : *dataList)
    {
        data->loadData();
    }
}

void DataManager::loadAllParamData()
{
    ESP_LOGD(name.c_str(), "Carregando dados parametrizados, %d registrados...", dataParamList.size());
    loadAllData(&dataParamList, &dataParamListMutex);
}

void DataManager::loadAllRuntimeData()
{
    ESP_LOGD(name.c_str(), "Carregando dados runtime, %d registrados...", dataRuntimeList.size());
    loadAllData(&dataRuntimeList, &dataRuntimeListMutex);
}

uint8_t DataManager::getRegistredParamDataCount()
{
    ESP_LOGD(name.c_str(), "Quantidade de dados parametrizados registrados: %d", dataParamList.size());
    return dataParamList.size();
}