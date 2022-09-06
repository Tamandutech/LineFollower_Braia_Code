#ifndef GRAPH_COMPONENT_H
#define GRAPH_COMPONENT_H

#include <string>
#include <mutex>
#include <vector>
#include <utility>

#include "DataAbstract.hpp"
#include "cJSON.h"

#include "esp_log.h"

class GraphComponent
{
    public:
        GraphComponent(std::string graphName, int buffsize, int plotsnum);
        DataAbstract<int16_t> *buffsize;
        std::pair<float,float> getPoint(int index, int plotid);
        void addPoint(std::pair<float,float> point);
        void addValue(float value,int plotid);
        void removePoint(int index, int plotid, bool removeTime = true);
        void clearPlot(int plotid);
        void clearTime();
        void clearAllPlots();
        size_t plotSize(int plotid);
        std::string graphDataStringfyPlot(int plotid);
        std::string graphDataStringfyAllPlots();
    private:
        void removePointof_AllPlots(int index);
        void _removePoint(int index, int plotid, bool removeTime);
        std::vector<std::vector<float>> datagraph;
        std::mutex datagraphMutex;
        std::vector<float> timedata;
        std::mutex timedataMutex;
        std::string graphName;
        DataAbstract<int16_t> *plotsnum;
};
#endif