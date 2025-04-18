#ifndef METRO_LINE_H
#define METRO_LINE_H

#include <string>
#include <memory>
#include <map>
#include "MetroStation.h"
#include "json.hpp"

using json = nlohmann::json;

class MetroLine {
public:
    std::string name;
    std::shared_ptr<MetroStation> startStation;
    bool isOneWay;
    bool isPurple;
    std::map<std::string, std::shared_ptr<MetroStation>> routeHeads;

    MetroLine(const json& lineData);

    std::shared_ptr<MetroStation> getStation(const std::string& name);
    int getStationCount(const std::string& routeType);
};

#endif