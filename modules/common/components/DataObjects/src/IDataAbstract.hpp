
#ifndef IDATA_ABSTRACT_HPP
#define IDATA_ABSTRACT_HPP

#include <iostream>
#include <typeinfo>
#include <cxxabi.h>
#include <atomic>
#include <memory>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>

class IDataAbstract
{
public:
    IDataAbstract(std::string name, std::string parentObjectName)
    {
        // armazena nome da variável para serialização e LOGs.
        this->name = parentObjectName + "." + name;
    }

    virtual ~IDataAbstract(){};

    // virtual void serialize(const char *buffer) = 0;
    // virtual void deserialize(const char *data) = 0;

    virtual void saveData() = 0;
    virtual void loadData() = 0;

    virtual void setStreamInterval(uint32_t interval) = 0;
    virtual uint32_t getStreamInterval() = 0;
    virtual void setStreamTime(uint32_t streamTime) = 0;
    virtual uint32_t getStreamTime() = 0;
    virtual uint32_t getLastChange() = 0;

    virtual void setData(std::string data) = 0;
    virtual std::string getDataString(std::string ctrl = "") = 0;

    std::string getName()
    {
        return this->name;
    };

protected:
    std::string name;

    std::string demangle(const char *mangled)
    {
        int status;
        std::unique_ptr<char[], void (*)(void *)> result(
            abi::__cxa_demangle(mangled, 0, 0, &status), std::free);
        return result.get() ? std::string(result.get()) : "error occurred";
    };
};

#endif