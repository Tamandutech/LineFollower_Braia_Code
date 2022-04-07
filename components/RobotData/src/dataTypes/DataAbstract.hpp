#ifndef DATA_ABSTRACT_HPP
#define DATA_ABSTRACT_HPP

#include <iostream>
#include <typeinfo>
#include <cxxabi.h>
#include <atomic>
#include <memory>
#include <cstdlib>
#include <string>
#include <vector>

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include "esp_log.h"

#include "dataEnums.h"

#include "DataStorage.hpp"

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

template <class T>
class DataAbstract_spec
{
public:
    T getData();
    void setData(T data);

    void saveData(std::string name);
    void loadData(std::string name);

protected:
    DataAbstract_spec(T value);
    virtual ~DataAbstract_spec();
    std::atomic<T> *data;

private:
};

template <class T>
class DataAbstract : public DataAbstract_spec<T>
{
public:
    DataAbstract(std::string name);
    DataAbstract(std::string name, T value);
    virtual ~DataAbstract();

protected:
    std::string demangle(const char *mangled);

private:
    std::string name;
};

#include "DataAbstract.cpp"
#endif