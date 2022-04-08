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
class DataAbstract
{
public:
    DataAbstract(std::string name, std::string parentObjectName);
    DataAbstract(std::string name, std::string parentObjectName, T value);
    virtual ~DataAbstract();

    T getData();
    void setData(T data);

    void saveData();
    void loadData();

protected:
private:
    std::string name;
    std::atomic<T> *data;

    DataStorage *storage;

    std::string demangle(const char *mangled);
};

#include "DataAbstract.cpp"
#endif