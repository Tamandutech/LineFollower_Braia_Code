#ifndef DATA_MAP_CPP
#define DATA_MAP_CPP

#include "DataMap.hpp"

std::mutex DataMap::mapDataListMutex;

DataMap::DataMap(std::string name, std::string parentObjectName) : IDataAbstract(name, parentObjectName)
{
    ESP_LOGD(name.c_str(), "Criando *dado do tipo DataMap.");

    this->dataStorage = dataStorage->getInstance();
    this->dataManager = dataManager->getInstance();

    mapDataList.clear();
}

DataMap::~DataMap()
{
}

MapData DataMap::getData(uint8_t posicao)
{
    std::lock_guard<std::mutex> myLock(mapDataListMutex);

    if (mapDataList.size() <= posicao)
    {
        ESP_LOGE(this->name.c_str(), "Não há dados nessa posição.");
        return MapData();
    }

    // seta o valor do dado na lista
    auto itList = this->mapDataList.begin();
    std::advance(itList, posicao);

    return *itList;
}

void DataMap::newData(MapData mapData)
{
    std::lock_guard<std::mutex> myLock(mapDataListMutex);

    this->mapDataList.push_back(mapData);
}

void DataMap::newData(std::string mapData)
{
    ESP_LOGD(this->name.c_str(), "Adicionando novo mapeamento na lista: %s", mapData.c_str());

    // separar dados por virgula
    std::vector<std::string> mapDataVector;

    std::stringstream ss(mapData);
    std::string s;

    ESP_LOGD(this->name.c_str(), "Separando dados por virgula...");

    while (std::getline(ss, s, ','))
    {
        ESP_LOGD(this->name.c_str(), "Dado: %s", s.c_str());
        mapDataVector.push_back(s);
    }

    if (mapDataVector.size() < 4)
    {
        ESP_LOGE(this->name.c_str(), "Dados inválidos. Quantidade de dados: %d", mapDataVector.size());
        return;
    }

    MapData mapDataTemp;
    // setar dados na struct
    mapDataTemp.MapTime = std::stoi(mapDataVector[1]);
    mapDataTemp.MapEncMedia = std::stoi(mapDataVector[2]);
    //mapDataTemp.MapEncLeft = std::stoi(mapDataVector[3]);
    //mapDataTemp.MapEncRight = std::stoi(mapDataVector[4]);
    //mapDataTemp.MapStatus = std::stoi(mapDataVector[5]);
    mapDataTemp.MapTrackStatus = std::stoi(mapDataVector[3]);
    mapDataTemp.MapOffset = std::stoi(mapDataVector[4]);

    this->newData(mapDataTemp);
}

std::string DataMap::getDataString()
{
    return this->getDataString("0");
}

std::string DataMap::getDataString(std::string ctrl)
{
    uint8_t posicao = ctrl.size() > 0 ? std::stoi(ctrl) : 0;

    std::lock_guard<std::mutex> myLock(mapDataListMutex);

    if (mapDataList.size() <= posicao)
    {
        ESP_LOGE(this->name.c_str(), "Não há dados nessa posição.");
        return "";
    }

    // avança na lista
    auto itList = this->mapDataList.begin();
    std::advance(itList, posicao);

    std::string line;
    line = std::to_string(posicao) + "," + std::to_string(itList->MapTime) + "," + 
    std::to_string(itList->MapEncMedia) + "," + std::to_string(itList->MapTrackStatus) + "," 
    + std::to_string(itList->MapOffset);

    ESP_LOGD(this->name.c_str(), "Dados: %s", line.c_str());

    return line;
}

void DataMap::setData(uint8_t posicao, MapData data)
{
    std::lock_guard<std::mutex> myLock(mapDataListMutex);

    if (mapDataList.size() <= posicao)
    {
        ESP_LOGE(this->name.c_str(), "Não há dados nessa posição.");
        return;
    }

    // seta o valor do dado na lista
    auto itList = this->mapDataList.begin();
    std::advance(itList, posicao);

    //itList->MapEncLeft = data.MapEncLeft;
    //itList->MapEncRight = data.MapEncRight;
    itList->MapEncMedia = data.MapEncMedia;
    //itList->MapStatus = data.MapStatus;
    itList->MapTime = data.MapTime;
    itList->MapTrackStatus = data.MapTrackStatus;
    itList->MapOffset = data.MapOffset;
}

void DataMap::setData(std::string data)
{
    if (data.at(0) == 'n')
        return newData(data);

    ESP_LOGD(this->name.c_str(), "Definindo mapeamento já existente.");

    // separa os dados
    std::vector<std::string> dataList;

    std::stringstream ss(data);
    std::string s;
    while (std::getline(ss, s, ','))
    {
        dataList.push_back(s);
    }

    if (dataList.size() < 5)
    {
        ESP_LOGE(this->name.c_str(), "Erro ao setar dados do tipo DataMap. Entrada inválida.");
        return;
    }

    MapData tempMapData;

    // seta o valor do dado na lista
    tempMapData.MapTime = stoi(dataList[1]);
    tempMapData.MapEncMedia = stoi(dataList[2]);
    //tempMapData.MapEncLeft = stoi(dataList[3]);
    //tempMapData.MapEncRight = stoi(dataList[4]);
    //tempMapData.MapStatus = stoi(dataList[5]);
    tempMapData.MapTrackStatus = stoi(dataList[3]);
    tempMapData.MapOffset = stoi(dataList[4]);

    setData(stoi(dataList[0]), tempMapData);
}

void DataMap::saveData()
{
    ESP_LOGD(this->name.c_str(), "Salvando todas as linhas de dataMap.");

    mapDataListMutex.lock();

    size_t listSize = this->mapDataList.size();

    if (listSize < 200)
        ESP_LOGD(this->name.c_str(), "Quantidade de linhas: %d", listSize);
    else
    {
        ESP_LOGE(this->name.c_str(), "Quantidade de linhas acima do esperado: %d", listSize);
        return;
    }

    // // Armazena a quantidade de itens
    // dataStorage->save_data(this->name, (char *)&listSize, sizeof(listSize));

    dataStorage->delete_data(this->name);

    // Armazena os dados fazendo append de cada struct
    int16_t sizeMap = this->getSize()*sizeof(MapData);
    char * dataSave = (char*) malloc(sizeMap);
    int16_t i = 0;
    for (auto &mapData : this->mapDataList)
    {
        memcpy(dataSave + i, &mapData, sizeof(MapData));
        i += sizeof(MapData);
        ESP_LOGD(this->name.c_str(), "Serializando mapData: %lu, %ld, %u, %d", mapData.MapTime, mapData.MapEncMedia,  mapData.MapTrackStatus, mapData.MapOffset);
    }
    dataStorage->save_data(this->name, dataSave, sizeMap, "ab");
    free(dataSave);

    mapDataListMutex.unlock();

    ESP_LOGD(this->name.c_str(), "Dados de mapeamento serializados para o storage.");
}

void DataMap::loadData()
{
    char *data = NULL;
    size_t size = 0;

    dataStorage->load_data(this->name, &data, &size);

    if (size == 0)
    {
        ESP_LOGE(this->name.c_str(), "Erro ao carregar dados do tipo DataMap. Nenhum dado encontrado.");
        return;
    }

    if (size > 200 * sizeof(MapData))
    {
        ESP_LOGE(this->name.c_str(), "Erro ao carregar dados do tipo DataMap. Tamanho do dado maior que o esperado.");
        return;
    }

    mapDataListMutex.lock();
    this->mapDataList.clear();
    mapDataListMutex.unlock();

    ESP_LOGD(this->name.c_str(), "Carregando dados de Mapeamento, tamanho do buffer: %d bytes", size);

    if (data)
    {
        // percorre todo o arquivo
        for (size_t i = 0; i < size; i += sizeof(MapData))
        {
            MapData tempMapData;
            memcpy(&tempMapData, data + i, sizeof(MapData));

            ESP_LOGD(this->name.c_str(), "Deserializando mapData: %lu, %ld, %u, %d", tempMapData.MapTime, tempMapData.MapEncMedia, tempMapData.MapTrackStatus, tempMapData.MapOffset);

            mapDataListMutex.lock();
            this->mapDataList.push_back(tempMapData);
            mapDataListMutex.unlock();
        }

        free(data);

        ESP_LOGD(this->name.c_str(), "Dados de mapeamento carregados do storage.");
    }
}

uint8_t DataMap::getSize()
{
    return this->mapDataList.size();
}

void DataMap::clearAllData()
{
    mapDataListMutex.lock();
    this->mapDataList.clear();
    mapDataListMutex.unlock();
}
void DataMap::clearData(uint8_t pos)
{
    mapDataListMutex.lock();
    if (mapDataList.size() <= pos)
    {
        ESP_LOGE(this->name.c_str(), "Não há dados nessa posição.");
        return;
    }
    auto it = this->mapDataList.begin();
    std::advance(it, pos);
    this->mapDataList.erase(it);
    mapDataListMutex.unlock();
}

void DataMap::setStreamInterval(uint32_t interval)
{
    this->stream_interval.store(interval, std::memory_order_release);
}

uint32_t DataMap::getStreamInterval()
{
    return this->stream_interval.load(std::memory_order_acquire);
}


void DataMap::setStreamTime(uint32_t streamTime)
{
    this->stream_time.store(streamTime, std::memory_order_release);
}

uint32_t DataMap::getStreamTime()
{
    return this->stream_time.load(std::memory_order_acquire);
}

uint32_t DataMap::getLastChange()
{
    return this->time_last_change.load(std::memory_order_acquire);
}

#endif