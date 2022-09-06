#include "GraphComponent.hpp"
GraphComponent::GraphComponent(std::string graphName, int buffsize, int plotsnum)
{
    this->graphName = graphName;
    this->buffsize = new DataAbstract<int16_t>("buffer",graphName,buffsize);
    this->plotsnum = new DataAbstract<int16_t>("plotsnum",graphName,plotsnum);
    datagraphMutex.lock();
    datagraph.resize(plotsnum);
    datagraphMutex.unlock();
}

void GraphComponent::addPoint(std::pair<float,float> point)
{
    datagraphMutex.lock();
    timedataMutex.lock();
    while(datagraph.at(0).size() >= buffsize->getData())
    {
        ESP_LOGD(graphName.c_str(), "Buffer lotado, removendo pontos em excesso");
        removePointof_AllPlots(0);
    }
    datagraph.at(0).push_back(point.first);
    timedata.push_back(point.second);
    ESP_LOGD(graphName.c_str(), "Ponto adicionado (valor: %.2f, tempo: %.2f)", point.first,point.second);
    datagraphMutex.unlock();
    timedataMutex.unlock();
}

void GraphComponent::addValue(float value,int plotid)
{
    datagraphMutex.lock();
    timedataMutex.lock();
    if(datagraph.at(plotid).size() < timedata.size())
    {
        datagraph.at(plotid).push_back(value);
        ESP_LOGD(graphName.c_str(), "Ponto adicionado (valor: %.2f)", value);

    }
    else ESP_LOGD(graphName.c_str(), "Plot %d cheio, adicione novos pontos", plotid);
    datagraphMutex.unlock();
    timedataMutex.unlock();
}

 std::pair<float,float> GraphComponent::getPoint(int index, int plotid)
 {
    datagraphMutex.lock();
    timedataMutex.lock();
    float data = datagraph.at(plotid).at(index);
    float time = timedata.at(index);
    datagraphMutex.unlock();
    timedataMutex.unlock();
    ESP_LOGD(graphName.c_str(), "Ponto lido (valor: %.2f, tempo: %.2f)", data,time);
    return std::make_pair(data,time);
 }

void GraphComponent::removePoint(int index, int plotid, bool removeTime)
{
    datagraphMutex.lock();
    timedataMutex.lock();
    _removePoint(index,plotid,removeTime);
    datagraphMutex.unlock();
    timedataMutex.unlock();
}

void GraphComponent::_removePoint(int index, int plotid, bool removeTime)
{
    auto data = datagraph.at(plotid).begin() + index;
    ESP_LOGD(graphName.c_str(), "Ponto a ser removido(valor: %.2f)",datagraph.at(plotid).at(index));
    datagraph.at(plotid).erase(data);
    if(removeTime)
    {
        ESP_LOGD(graphName.c_str(), "Ponto a ser removido(time: %.2f)",timedata.at(index));
        auto time = timedata.begin() + index;
        timedata.erase(time);
    }
     ESP_LOGD(graphName.c_str(), "Ponto removido com sucesso");
}

void GraphComponent::removePointof_AllPlots(int index)
{
    _removePoint(index,0,true);
    for (int plotnum = 1; plotnum < plotsnum->getData(); plotnum++)
    {
        _removePoint(index,plotnum,false);
    }

}
std::string GraphComponent::graphDataStringfyPlot(int plotid)
{
    datagraphMutex.lock();
    timedataMutex.lock();
    cJSON *points = cJSON_CreateArray();
    for(int i = 0;i < datagraph.at(plotid).size(); i++)
    {
        cJSON *point = cJSON_CreateObject();
        cJSON_AddItemToArray(points,point);
        cJSON *data = cJSON_CreateNumber(datagraph.at(plotid).at(i));
        cJSON *time = cJSON_CreateNumber(timedata.at(i));
        cJSON_AddItemToObject(point,"data",data);
        cJSON_AddItemToObject(point,"time",time);
    }
    datagraphMutex.unlock();
    timedataMutex.unlock();
    std::string JsonPoints = cJSON_Print(points);
    ESP_LOGD(graphName.c_str(), "Json gerado para o plot %d: %s",plotid, JsonPoints.c_str());
    cJSON_Delete(points);
    return JsonPoints;

}

std::string GraphComponent::graphDataStringfyAllPlots()
{
    datagraphMutex.lock();
    timedataMutex.lock();
    cJSON *points = cJSON_CreateArray();
    for(int i = 0;i < datagraph.at(0).size(); i++)
    {
        cJSON *point = cJSON_CreateObject();
        cJSON_AddItemToArray(points,point);
        for(int plots = 0; plots < plotsnum->getData(); plots++)
        {
            cJSON *data = cJSON_CreateNumber(datagraph.at(plots).at(i));
            cJSON_AddItemToObject(point,("data" + std::to_string(plots)).c_str(),data);
        }
        cJSON *time = cJSON_CreateNumber(timedata.at(i));
        cJSON_AddItemToObject(point,"time",time);
    }
    datagraphMutex.unlock();
    timedataMutex.unlock();
    std::string JsonPoints = cJSON_Print(points);
    ESP_LOGD(graphName.c_str(), "Json gerado para todos os plots juntos: %s",JsonPoints.c_str());
    cJSON_Delete(points);
    return JsonPoints;

}

void GraphComponent::clearAllPlots()
{
    datagraphMutex.lock();
    timedataMutex.lock();
    for(int i=0; i < plotsnum->getData();i++)
    {
        ESP_LOGD(graphName.c_str(), "Removendo plot %d", i);
        datagraph.at(i).clear();
        ESP_LOGD(graphName.c_str(), "Plot %d removido com sucesso", i);
    }
    timedata.clear();
    ESP_LOGD(graphName.c_str(), "Todos os Plots foram removidos com sucesso");
    datagraphMutex.unlock();
    timedataMutex.unlock();
}

void GraphComponent::clearPlot(int plotid)
{
    datagraphMutex.lock();
    datagraph.at(plotid).clear();
    ESP_LOGD(graphName.c_str(), "Plot %d removido com sucesso",plotid);
    datagraphMutex.unlock();
}

void GraphComponent::clearTime()
{
    timedataMutex.lock();
    timedata.clear();
    ESP_LOGD(graphName.c_str(), "Time array limpo com sucesso");
    timedataMutex.unlock();
}

size_t GraphComponent::plotSize(int plotid)
{
    datagraphMutex.lock();
    size_t tam = datagraph.at(plotid).size();
    ESP_LOGD(graphName.c_str(), "Tamanho do Plot %d: %d pontos",plotid,tam);
    datagraphMutex.unlock();
    return tam;
}