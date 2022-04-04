#ifndef DATA_ABSTRACT_HPP
#define DATA_ABSTRACT_HPP

#include <iostream>
#include <typeinfo>
#include <atomic>
#include <cxxabi.h>
#include <memory>
#include <cstdlib>
#include <string>

#include "esp_log.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

template <class T>
class DataAbstract
{
public:
    // construtor da classe
    DataAbstract(std::string name)
    {
        ESP_LOGD(name.c_str(), "Criando *dado do tipo %s", demangle(typeid(*this).name()).c_str());

        // armazena nome da variável para serialização e LOGs.
        this->name = name;

        // inicializando o ponteiro para o tipo T
        this->data = new std::atomic<T>();
    }

    // destrutor da classe
    virtual ~DataAbstract()
    {
        ESP_LOGD(this->name.c_str(), "Destruindo dado do tipo %s", demangle(typeid(*this).name()).c_str());

        // deletando o ponteiro para o tipo T
        delete this->data;
    }

    // metodos de get e set
    T getData()
    {
        // retorna o valor do dado
        return this->data->load();
    }

    void setData(T data)
    {
        // seta o valor do dado
        this->data->store(data);
    }

    // metodos de serialização e deserialização
    void serialize(std::ostream &out)
    {
        // serializa o dado
        out << this->data->load();
    }
    void deserialize(std::istream &in)
    {
        // deserializa o dado
        in >> this->data->store();
    }

protected:
    std::string demangle(const char *mangled)
    {
        int status;
        std::unique_ptr<char[], void (*)(void *)> result(
            abi::__cxa_demangle(mangled, 0, 0, &status), std::free);
        return result.get() ? std::string(result.get()) : "error occurred";
    }

private:
    // dado
    std::atomic<T> *data;

    // nome do dado
    std::string name;
};

#endif