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

    itList->MapEncLeft = data.MapEncLeft;
    itList->MapEncRight = data.MapEncRight;
    itList->MapEncMedia = data.MapEncMedia;
    itList->MapStatus = data.MapStatus;
    itList->MapTime = data.MapTime;
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

    if (mapDataVector.size() < 5)
    {
        ESP_LOGE(this->name.c_str(), "Dados inválidos. Quantidade de dados: %d", mapDataVector.size());
        return;
    }

    MapData mapDataTemp;
    // setar dados na struct
    mapDataTemp.MapTime = std::stoi(mapDataVector[1]);
    mapDataTemp.MapEncMedia = std::stoi(mapDataVector[2]);
    mapDataTemp.MapEncLeft = std::stoi(mapDataVector[3]);
    mapDataTemp.MapEncRight = std::stoi(mapDataVector[4]);
    mapDataTemp.MapStatus = std::stoi(mapDataVector[5]);

    this->newData(mapDataTemp);
}

std::string DataMap::getDataString()
{
    return this->getDataString(0);
}

std::string DataMap::getDataString(uint8_t posicao)
{
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
    line = std::to_string(posicao) + "," + std::to_string(itList->MapTime) + "," + std::to_string(itList->MapEncMedia) + "," + std::to_string(itList->MapEncLeft) + "," + std::to_string(itList->MapEncRight) + "," + std::to_string(itList->MapStatus);

    return line;
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

    if (dataList.size() < 6)
    {
        ESP_LOGE(this->name.c_str(), "Erro ao setar dados do tipo DataMap. Entrada inválida.");
        return;
    }

    // avança na lista
    MapData tempMapData;

    // seta o valor do dado na lista
    tempMapData.MapTime = stoi(dataList[1]);
    tempMapData.MapEncMedia = stoi(dataList[2]);
    tempMapData.MapEncLeft = stoi(dataList[3]);
    tempMapData.MapEncRight = stoi(dataList[4]);
    tempMapData.MapStatus = stoi(dataList[5]);

    setData(stoi(dataList[0]), tempMapData);
}

void DataMap::saveData()
{
    ESP_LOGD(this->name.c_str(), "Salvando todas as linhas de dataMap.");

    std::string file;
    size_t listSize = 0;

    mapDataListMutex.lock();
    listSize = this->mapDataList.size();
    mapDataListMutex.unlock();

    ESP_LOGD(this->name.c_str(), "Quantidade de linhas: %d", listSize);

    for (int i = 0; i < listSize; i++)
    {
        file += this->getDataString(i) + "\n";
        ESP_LOGD(this->name.c_str(), "Gerando linha.");
    }

    ESP_LOGD(this->name.c_str(), "Salvando arquivo.");
    dataStorage->save_data(this->name, (char *)file.c_str(), (file.length() + 1));

    ESP_LOGD(this->name.c_str(), "Salvando dados de Mapeamento, tamanho do buffer: %d bytes", file.length() + 1);
}

void DataMap::loadData()
{
    char *data = new char;
    char *token;
    size_t size = 0;

    dataStorage->load_data(this->name, data, &size);

    if (size == 0)
    {
        ESP_LOGE(this->name.c_str(), "Erro ao carregar dados do tipo DataMap. Nenhum dado encontrado.");
        return;
    }

    ESP_LOGD(this->name.c_str(), "Carregando dados de Mapeamento, tamanho do buffer: %d bytes", size);

    mapDataListMutex.lock();
    this->mapDataList.clear();
    mapDataListMutex.unlock();

    token = strtok(data, "\n");

    while (token != NULL)
    {
        this->newData(token);

        token = strtok(NULL, "\n");
    }
}
#endif