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

    this->storage = storage->getInstance();
}

template <class T>
DataAbstract<T>::DataAbstract(std::string name, std::string parentObjectName, T value)
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo %s, com valor inicial definido: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(value)).c_str());

    this->name = parentObjectName + "." + name;

    this->data = new std::atomic<T>();

    this->storage = storage->getInstance();
}

template <class T>
DataAbstract<T>::~DataAbstract()
{
    ESP_LOGD(this->name.c_str(), "Destruindo dado do tipo %s", demangle(typeid(*this).name()).c_str());
    delete this->data;
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

    FILE *f = fopen(("/robotdata/" + name).c_str(), "wb");
    ESP_LOGD(name.c_str(), "Abrindo arquivo %s", name.c_str());

    if (f == NULL)
    {
        ESP_LOGE(name.c_str(), "Falha ao abrir arquivo para escrita");
        return;
    }

    size_t bytes = fwrite(&temp, sizeof(T), 1, f);
    ESP_LOGD(name.c_str(), "Escrito %s, %d bytes", name.c_str(), bytes);

    fclose(f);
}

template <class T>
void DataAbstract<T>::loadData()
{
    T temp = T();

    FILE *f = fopen(("/robotdata/" + name).c_str(), "r");
    ESP_LOGD(name.c_str(), "Abrindo arquivo %s", name.c_str());

    if (f == NULL)
    {
        ESP_LOGE(name.c_str(), "Falha ao abrir arquivo para leitura");
        return;
    }

    size_t bytes = fread(&temp, sizeof(T), 1, f);
    ESP_LOGD(name.c_str(), "Lido %s, %d bytes", name.c_str(), bytes);

    this->data->store(temp);

    fclose(f);
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