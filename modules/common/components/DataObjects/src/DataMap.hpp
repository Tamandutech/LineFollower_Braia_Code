#ifndef DATA_MAP_HPP
#define DATA_MAP_HPP

#include <iostream>
#include <typeinfo>
#include <cxxabi.h>
#include <atomic>
#include <memory>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <list>
#include <iterator>

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include "IDataAbstract.hpp"

#include "DataStorage.hpp"
#include "DataManager.hpp"

#include "esp_log.h"

struct MapData
{
    uint32_t MapTime;
    int32_t MapEncMedia;
    int32_t MapEncLeft;
    int32_t MapEncRight;
    uint8_t MapStatus;
    uint8_t MapTrackStatus;
};

class DataMap : public IDataAbstract
{
public:
    DataMap(std::string name, std::string parentObjectName);
    virtual ~DataMap();

    MapData getData(uint8_t posicao);
    void setData(uint8_t posicao, MapData mapData);

    void newData(MapData mapData);
    void newData(std::string mapData);

    /// @brief Definir dado estruturado de mapeamento
    /// @param mapData Dados da struct separados por ",". Ex.: "Posicao,MapTime,MapEncMedia,MapEncLeft,MapEncRight,MapStatus"
    void setData(std::string data);

    /// @brief Obter o primeiro dado da lista em formato string
    /// @return Dados da struct separados por ",". Ex.: "Posicao,MapTime,MapEncMedia,MapEncLeft,MapEncRight,MapStatus"
    std::string getDataString();

    /// @brief Obter dado na posição especificada da lista em formato string
    /// @param ctrl Posição da lista
    /// @return Dados da struct separados por ",". Ex.: "Posicao,MapTime,MapEncMedia,MapEncLeft,MapEncRight,MapStatus"
    std::string getDataString(std::string ctrl);

    std::uint8_t getSize();

    void saveData();
    void loadData();

    void clearAllData();
    void clearData(uint8_t pos);

protected:
private:
    DataStorage *dataStorage;
    DataManager *dataManager;

    std::list<MapData> mapDataList;
    static std::mutex mapDataListMutex;
};

#endif