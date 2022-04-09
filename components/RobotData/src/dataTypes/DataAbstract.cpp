#ifndef DATA_ABSTRACT_CPP
#define DATA_ABSTRACT_CPP

#include "DataAbstract.hpp"

template <class T>
DataAbstract<T>::DataAbstract(std::string name, std::string parentObjectName)
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo %s, variavel inicializada com: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(T())).c_str());

    // armazena nome da variável para serialização e LOGs.
    this->name = parentObjectName + "." + name;

    this->data = new std::atomic<T>();

    this->dataStorage = dataStorage->getInstance();
}

template <class T>
DataAbstract<T>::DataAbstract(std::string name, std::string parentObjectName, T value)
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo %s, com valor inicial definido: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(value)).c_str());

    this->name = parentObjectName + "." + name;

    this->data = new std::atomic<T>();

    this->dataStorage = dataStorage->getInstance();
}

template <class T>
DataAbstract<T>::~DataAbstract()
{
    ESP_LOGD(this->name.c_str(), "Destruindo dado do tipo %s", demangle(typeid(*this).name()).c_str());
    delete this->data;
}

template <class T>
void DataAbstract<T>::serialize(const char *buffer)
{
    ESP_LOGD(this->name.c_str(), "Serializando dado do tipo %s", demangle(typeid(*this).name()).c_str());

    T tempT = this->getData();
    memcpy((char *)&tempT, (char *)buffer, sizeof(T));
}

template <class T>
void DataAbstract<T>::deserialize(const char *data)
{
    ESP_LOGD(this->name.c_str(), "Deserializando dado do tipo %s, valor: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(*data)).c_str());

    T tempT = T();
    memcpy((char *)data, (char *)&tempT, sizeof(T));

    this->setData(tempT);
}

template <class T>
T DataAbstract<T>::getData()
{
    // retorna o valor do dado
    return this->data->load();
}

template <class T>
void DataAbstract<T>::setData(T data)
{
    // seta o valor do dado
    this->data->store(data);
}

template <class T>
void DataAbstract<T>::saveData()
{
    T temp = this->data->load();
    dataStorage->save_data(this->name, (char *)&temp, sizeof(T));
}

template <class T>
void DataAbstract<T>::loadData()
{
    T temp = T();
    dataStorage->load_data(this->name, (char *)&temp, sizeof(T));
    this->data->store(temp);
}

template <class T>
std::string DataAbstract<T>::getName()
{
    return this->name;
}

template <class T>
std::string DataAbstract<T>::demangle(const char *mangled)
{
    int status;
    std::unique_ptr<char[], void (*)(void *)> result(
        abi::__cxa_demangle(mangled, 0, 0, &status), std::free);
    return result.get() ? std::string(result.get()) : "error occurred";
}

#endif