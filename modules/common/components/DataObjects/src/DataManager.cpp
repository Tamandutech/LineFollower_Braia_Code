#include "DataManager.hpp"

std::atomic<DataManager *> DataManager::instance;
std::mutex DataManager::instanceMutex;
std::string DataManager::name;

std::vector<IDataAbstract *> DataManager::dataParamList;
std::mutex DataManager::dataParamListMutex;

std::vector<IDataAbstract *> DataManager::dataRuntimeList;
std::mutex DataManager::dataRuntimeListMutex;

std::vector<IDataAbstract *> DataManager::dataStreamList;
std::mutex DataManager::dataStreamListMutex;

DataManager::DataManager()
{
    this->name = "DataManager";
    //esp_log_level_set("DataManager",ESP_LOG_DEBUG);
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

void DataManager::registerParamDataChanged(std::string name)
{
    ESP_LOGD(this->name.c_str(), "Obtendo parametro: %s", name.c_str());
    IDataAbstract *param = nullptr;
    bool paramRegistered = false;

    dataParamChangedListMutex.lock();
    for(auto data : dataParamChangedList)
    {
        if (data->getName() == name) paramRegistered = true;
    }
    dataParamChangedListMutex.unlock();

    if(!paramRegistered)
    {
        for (auto data : dataParamList)
        {
            if (data->getName() == name)
            {
                param = data;
            }
        }
        if(param != nullptr)
        {
            ESP_LOGD(name.c_str(), "Registrando dado parametrizado alterado não salvo: %s (%p)", param->getName().c_str(), param);
            registerData(param, &dataParamChangedList, &dataParamChangedListMutex);
        }
    }
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
        vTaskDelay(0);
    }
}

void DataManager::saveAllParamData()
{
    ESP_LOGD(name.c_str(), "Salvando dados parametrizados, %d registrados...", dataParamList.size());
    saveAllData(&dataParamList, &dataParamListMutex);
}

void DataManager::saveAllParamDataChanged()
{
    ESP_LOGD(name.c_str(), "Salvando dados parametrizados alterados, %d registrados...", dataParamChangedList.size());
    saveAllData(&dataParamChangedList, &dataParamChangedListMutex);
    std::lock_guard<std::mutex> myLock(dataParamChangedListMutex);
    dataParamChangedList.clear();
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

std::vector<IDataAbstract *>::iterator DataManager::getDataListIterator(IDataAbstract *data, std::vector<IDataAbstract *> *dataList, std::mutex *dataListMutex)
{
    std::lock_guard<std::mutex> myLock(*dataListMutex);
    std::vector<IDataAbstract *>::iterator it;
    for (it = dataList->begin();it != dataList->end(); it++)
    {
        if (data->getName() == (*it)->getName())
        {
            return it;
        }
    }
    return it;
}
void DataManager::setParam(std::string name, std::string value, bool savedata)
{
    if(value.find("!") !=-1) value.erase(value.begin());
    ESP_LOGD(this->name.c_str(), "Setando parametro: %s = %s", name.c_str(), value.c_str());
    for (auto data : dataParamList)
    {
        if (data->getName() == name)
        {
            data->setData(value);
            if(savedata) data->saveData();
            return;
        }
    }

    ESP_LOGE(this->name.c_str(), "Parametro %s não encontrado.", name.c_str());
}

std::string DataManager::getParam(std::string name, std::string ctrl)
{
    ESP_LOGD(this->name.c_str(), "Obtendo parametro: %s", name.c_str());
    for (auto data : dataParamList)
    {
        if (data->getName() == name)
        {
            return data->getDataString(ctrl);
        }
    }

    ESP_LOGE(this->name.c_str(), "Parametro %s não encontrado.", name.c_str());
    return "NOK: Parametro não encontrado.";
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

std::string DataManager::listRegistredParamData()
{
    uint8_t qtd = getRegistredParamDataCount();

    std::string list = "";

    list = "Dados parametrizados registrados: " + std::to_string(qtd) + '\n';

    ESP_LOGD(name.c_str(), "Listando dados parametrizados registrados: %d", qtd);

    for (uint8_t i = 0; i < qtd; i++)
    {
        list += " " + std::to_string(i) + " - " + dataParamList[i]->getName() + ": " + dataParamList[i]->getDataString() + '\n';
        ESP_LOGD(name.c_str(), "Dado %d (%p) -> %s: %s", i, dataParamList[i], dataParamList[i]->getName().c_str(), dataParamList[i]->getDataString().c_str());
    }

    ESP_LOGD(name.c_str(), "Retornando a lista.");

    return list;
}

void DataManager::setStreamInterval(std::string name, uint32_t time_in_ms)
{
    std::lock_guard<std::mutex> myLock(dataRuntimeListMutex);
    for (auto data : dataRuntimeList)
    {
        if (data->getName() == name)
        {
            data->setStreamInterval(time_in_ms);
            data->setStreamTime(0);
            auto it = this->getDataListIterator(data, &dataStreamList, &dataStreamListMutex);
            std::lock_guard<std::mutex> myLock(dataStreamListMutex);
            if(time_in_ms == 0) 
            {
                if(it != dataStreamList.end()) dataStreamList.erase(it);
            }
            else 
            {
                if(it == dataStreamList.end()) dataStreamList.push_back(data);
            }
        }
    }

}

int DataManager::NumItemsReadyStream()
{
    uint32_t time = xTaskGetTickCount()*portTICK_PERIOD_MS;
    std::lock_guard<std::mutex> myLock(dataStreamListMutex);
    int num = 0;
    for (auto data : dataStreamList)
    {
        ESP_LOGD(name.c_str(),"TempoStream: %lu, StreamTime: %lu",time,data->getStreamTime());
        if(time > data->getStreamTime())
        {
            num++;
        }
    }
    return num;
}

cJSON* DataManager::getStreamData()
{
    uint32_t time = xTaskGetTickCount()*portTICK_PERIOD_MS;
    cJSON* StreamObjects = cJSON_CreateArray();
    std::lock_guard<std::mutex> myLock(dataStreamListMutex);
    for (auto data : dataStreamList)
    {
        ESP_LOGD(name.c_str(),"Tempo: %lu, streamTime: %lu",time,data->getStreamTime());
        if(time > data->getStreamTime())
        {
            cJSON *Object = cJSON_CreateObject();
            cJSON_AddItemToArray(StreamObjects,Object);
            cJSON_AddStringToObject(Object,"name",data->getName().c_str());
            cJSON_AddStringToObject(Object,"value",data->getDataString().c_str());
            //cJSON *JsonInterval = cJSON_CreateNumber(data->getStreamInterval());
            //cJSON_AddItemToObject(Object,"interval", JsonInterval);
            cJSON *JsonTime = cJSON_CreateNumber(data->getLastChange());
            cJSON_AddItemToObject(Object,"Time", JsonTime);
            data->setStreamTime(time + data->getStreamInterval());
        }
    }
    char *debugStr = cJSON_Print(StreamObjects);
    ESP_LOGD(name.c_str(),"Json gerado para o stream: %s",debugStr);
    cJSON_free(debugStr);
    return StreamObjects;
}

void DataManager::setRuntime(std::string name, std::string value)
{
    ESP_LOGD(this->name.c_str(), "Setando Runtime Data: %s = %s", name.c_str(), value.c_str());
    std::lock_guard<std::mutex> myLock(dataRuntimeListMutex);
    for (auto data : dataRuntimeList)
    {
        if (data->getName() == name)
        {
            data->setData(value);
            return;
        }
    }
    ESP_LOGE(this->name.c_str(), "Runtime Data %s não encontrado.", name.c_str());
}

std::string DataManager::getRuntime(std::string name)
{
    ESP_LOGD(this->name.c_str(), "Obtendo Runtime Data: %s", name.c_str());
    std::lock_guard<std::mutex> myLock(dataRuntimeListMutex);
    for (auto data : dataRuntimeList)
    {
        if (data->getName() == name)
        {
            return data->getDataString();
        }
    }
    ESP_LOGE(this->name.c_str(), "Runtime Data %s não encontrado.", name.c_str());
    return "NOK: Runtime Data não encontrado.";
}

std::string DataManager::listRegistredRuntimeData()
{
    std::lock_guard<std::mutex> myLock(dataRuntimeListMutex);

    uint8_t qtd = dataRuntimeList.size();

    std::string list = "";

    list = "Dados em tempo de execução registrados: " + std::to_string(qtd) + '\n';

    ESP_LOGD(name.c_str(), "Listando dados runtime registrados: %d", qtd);

    for (uint8_t i = 0; i < qtd; i++)
    {
        list += " " + std::to_string(i) + " - " + dataRuntimeList[i]->getName() + ": " + dataRuntimeList[i]->getDataString() + '\n';
        ESP_LOGD(name.c_str(), "Dado %u (%p) -> %s: %s", i, dataRuntimeList[i], dataRuntimeList[i]->getName().c_str(), dataRuntimeList[i]->getDataString().c_str());
    }

    ESP_LOGD(name.c_str(), "Retornando a lista.");

    return list;
}