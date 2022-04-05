#include "DataAbstract.hpp"

// implementar o construtor
template <class T>
DataAbstract<T>::DataAbstract(std::string name)
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo %s", demangle(typeid(*this).name()).c_str());

    // armazena nome da variável para serialização e LOGs.
    this->name = name;

    // inicializando o ponteiro para o tipo T
    this->data = new std::atomic<T>();
}

template <class T>
DataAbstract<T>::DataAbstract(std::string name, T value)
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo %s, com valor inicial %s", demangle(typeid(*this).name()).c_str(), std::string(std::to_string(value)).c_str());

    // armazena nome da variável para serialização e LOGs.
    this->name = name;

    // inicializando o ponteiro para o tipo T
    this->data = new std::atomic<T>(value);
}

template <class T>
DataAbstract<T>::~DataAbstract()
{
    ESP_LOGD(this->name.c_str(), "Destruindo dado do tipo %s", demangle(typeid(*this).name()).c_str());

    // deletando o ponteiro para o tipo T
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
std::string DataAbstract<T>::demangle(const char *mangled)
{
    int status;
    std::unique_ptr<char[], void (*)(void *)> result(
        abi::__cxa_demangle(mangled, 0, 0, &status), std::free);
    return result.get() ? std::string(result.get()) : "error occurred";
}

template class DataAbstract<int16_t>;
template class DataAbstract<float>;
template class DataAbstract<bool>;
template class DataAbstract<CarState>;