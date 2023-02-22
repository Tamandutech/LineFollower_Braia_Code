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

    void setStreamInterval(uint32_t interval);
    uint32_t getStreamInterval();
    void setStreamTime(uint32_t streamTime);
    uint32_t getStreamTime();
    uint32_t getLastChange();

    std::string getName();
    std::string getDataString(std::string ctrl);

protected:
private:
    std::atomic<T> *data;
    std::atomic<uint32_t> time_last_change; // Tempo de execução em que o dado foi alterado pela última vez
    std::atomic<uint32_t> stream_interval; // Intervalo que o stream do atributo será feito
    std::atomic<uint32_t> stream_time; // momento em que o stream do atributo deve ser feito


    DataStorage *dataStorage;
    DataManager *dataManager;
};

#include "DataAbstract.cpp"
#endif