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

#include "dataEnums.h"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

template <class T>
class DataAbstract
{
public:
    // construtor da classe
    DataAbstract(std::string name);

    // construtor da classe com valor inicial
    DataAbstract(std::string name, T value);

    // destrutor da classe
    virtual ~DataAbstract();

    // metodos de get e set
    T getData();

    void setData(T data);

protected:
    std::string demangle(const char *mangled);

private:
    // dado
    std::atomic<T> *data;

    // nome do dado
    std::string name;
};

#endif