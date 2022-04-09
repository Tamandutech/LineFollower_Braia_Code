
#ifndef IDATA_ABSTRACT_HPP
#define IDATA_ABSTRACT_HPP

#include <string>

class IDataAbstract
{
public:
    virtual ~IDataAbstract(){};

    virtual void serialize(const char *buffer) = 0;
    virtual void deserialize(const char *data) = 0;

    virtual void saveData() = 0;
    virtual void loadData() = 0;
    virtual void setData(std::string data) = 0;
    virtual std::string getName() = 0;
};

#endif