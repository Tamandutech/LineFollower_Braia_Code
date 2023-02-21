#ifndef DATA_ABSTRACT_CPP
#define DATA_ABSTRACT_CPP

#include "DataAbstract.hpp"

template <class T>
DataAbstract<T>::DataAbstract(std::string name, std::string parentObjectName) : IDataAbstract(name, parentObjectName)
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo %s, variavel inicializada com: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(T())).c_str());

    this->dataStorage = dataStorage->getInstance();
    this->dataManager = dataManager->getInstance();

    this->data = new std::atomic<T>();
    this->time_last_change.store(0, std::memory_order_release);
    this->stream_interval.store(0, std::memory_order_release);
    this->stream_time.store(0, std::memory_order_release);
}

template <class T>
DataAbstract<T>::DataAbstract(std::string name, std::string parentObjectName, T value) : IDataAbstract(name, parentObjectName)
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo %s, com valor inicial definido: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(value)).c_str());

    this->dataStorage = dataStorage->getInstance();
    this->dataManager = dataManager->getInstance();

    this->data = new std::atomic<T>(value);
    this->time_last_change.store(0, std::memory_order_release);
    this->stream_interval.store(0, std::memory_order_release);
    this->stream_time.store(0, std::memory_order_release);
}

template <class T>
DataAbstract<T>::~DataAbstract()
{
    ESP_LOGD(this->name.c_str(), "Destruindo dado do tipo %s", demangle(typeid(*this).name()).c_str());
    delete this->data;
}

template <class T>
std::string DataAbstract<T>::getName()
{
    return this->name;
}

template <class T>
T DataAbstract<T>::getData()
{
    // retorna o valor do dado
    return this->data->load(std::memory_order_acquire);
}

template <class T>
std::string DataAbstract<T>::getDataString(std::string ctrl)
{
    // retorna o valor do dado
    return std::to_string(this->data->load(std::memory_order_acquire));
}

template <class T>
void DataAbstract<T>::setData(T data, uint32_t last_change)
{
    // seta o valor do dado
    this->data->store(data, std::memory_order_release);
    if(this->stream_interval.load(std::memory_order_acquire) != 0) this->time_last_change.store(last_change, std::memory_order_release);
}

template <class T>
void DataAbstract<T>::setData(std::string data, uint32_t last_change)
{
    float num = std::stof(data);
    this->data->store(num, std::memory_order_release);
    if(this->stream_interval.load(std::memory_order_acquire) != 0) this->time_last_change.store(last_change, std::memory_order_release);
    ESP_LOGD(this->name.c_str(), "Confirmando dado salvo: %s", std::to_string(this->data->load(std::memory_order_acquire)).c_str());
}

template <class T>
void DataAbstract<T>::saveData()
{
    T temp = this->data->load(std::memory_order_acquire);
    dataStorage->save_data(this->name, (char *)&temp, sizeof(T));
    ESP_LOGD(this->name.c_str(), "Salvando dado do tipo %s, valor: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(temp)).c_str());
}

template <class T>
void DataAbstract<T>::loadData()
{
    T temp = T();
    if (ESP_OK == dataStorage->load_data(this->name, (char *)&temp, sizeof(T)))
    {
        ESP_LOGD(this->name.c_str(), "Carregando dado do tipo %s, valor: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(temp)).c_str());
        this->data->store(temp, std::memory_order_release);
    }
}

template <class T>
void DataAbstract<T>::setStreamInterval(uint32_t interval)
{
    this->stream_interval.store(interval, std::memory_order_release);
}

template <class T>
uint32_t DataAbstract<T>::getStreamInterval()
{
    return this->stream_interval.load(std::memory_order_acquire);
}

template <class T>
void DataAbstract<T>::setStreamTime(uint32_t streamTime)
{
    this->stream_time.store(streamTime, std::memory_order_release);
}

template <class T>
uint32_t DataAbstract<T>::getStreamTime()
{
    return this->stream_time.load(std::memory_order_acquire);
}

template <class T>
uint32_t DataAbstract<T>::getLastChange()
{
    return this->time_last_change.load(std::memory_order_acquire);
}
#endif