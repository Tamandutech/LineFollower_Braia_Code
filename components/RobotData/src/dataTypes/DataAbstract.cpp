#ifndef DATA_ABSTRACT_CPP
#define DATA_ABSTRACT_CPP

#include "DataAbstract.hpp"

template <class T>
DataAbstract<T>::DataAbstract(std::string name) : DataAbstract_spec<T>(T())
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo %s, variavel inicializada com: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(T())).c_str());

    // armazena nome da variável para serialização e LOGs.
    this->name = name;
}

template <class T>
DataAbstract<T>::DataAbstract(std::string name, T value) : DataAbstract_spec<T>(value)
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo %s, com valor inicial definido: %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(value)).c_str());

    this->name = name;
}

template <class T>
DataAbstract<T>::~DataAbstract()
{
    ESP_LOGD(this->name.c_str(), "Destruindo dado do tipo %s", demangle(typeid(*this).name()).c_str());
}

template <class T>
DataAbstract_spec<T>::DataAbstract_spec(T value)
{
    this->data = new std::atomic<T>();
}

template <class T>
DataAbstract_spec<T>::~DataAbstract_spec()
{
    delete this->data;
}

template <class T>
T DataAbstract_spec<T>::getData()
{
    // retorna o valor do dado
    return this->data->load();
}

template <class T>
void DataAbstract_spec<T>::setData(T data)
{
    // seta o valor do dado
    this->data->store(data);
}

template <class T>
void DataAbstract_spec<T>::saveData(std::string name)
{
    T temp = this->data->load();

    FILE *f = fopen(("/robotdata/" + name).c_str(), "wb");
    ESP_LOGD(name.c_str(), "Abrindo arquivo %s", name.c_str());

    if (f == NULL)
    {
        ESP_LOGE(name.c_str(), "Failed to open file for writing");
        return;
    }

    size_t bytes = fwrite(&temp, sizeof(T), 1, f);
    ESP_LOGD(name.c_str(), "Escrito %s, %d bytes", name.c_str(), bytes);

    fclose(f);
}

template <class T>
void DataAbstract_spec<T>::loadData(std::string name)
{
    char *temp = new char[sizeof(T)];

    FIL *fp;
    f_open(fp, name.c_str(), FA_CREATE_NEW | FA_WRITE | FA_READ);

    if (fp == NULL)
        return;

    // ler dados do arquivo
    FRESULT result = f_read(fp, temp, sizeof(T), NULL);
    ESP_LOGD(name.c_str(), "Lendo %s. Resultado: %d", name.c_str(), result);

    ESP_LOGD(name.c_str(), "Carregando valor: %s", std::string(std::to_string(*(T *)temp)).c_str());
    this->data->store(*(T *)temp);
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