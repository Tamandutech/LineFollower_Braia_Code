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
#include <sstream>

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include "IDataAbstract.hpp"
#include "DataStorage.hpp"
#include "DataManager.hpp"

#include "esp_log.h"

template <class T>
class DataAbstract : public IDataAbstract
{
public:
    DataAbstract(std::string name, std::string parentObjectName);
    DataAbstract(std::string name, std::string parentObjectName, T value);
    virtual ~DataAbstract();

    T getData();
    void setData(T data);
    void setData(std::string data);

    void saveData();
    void loadData();

    std::string getName();
    std::string getDataString(std::string ctrl);

protected:
private:
    std::atomic<T> *data;

    DataStorage *dataStorage;
    DataManager *dataManager;
};

#include "DataAbstract.cpp"
#endif