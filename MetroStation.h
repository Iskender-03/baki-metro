#ifndef METRO_STATION_H
#define METRO_STATION_H

#include <string>
#include <memory>
#include <mutex>
#include <map>

class MetroStation {
public:
    std::string name;
    std::shared_ptr<std::timed_mutex> stationMutex;
    bool isOccupiedForward;
    bool isOccupiedBackward;
    bool isOneWay;

    std::map<std::string, std::shared_ptr<MetroStation>> nextStations;
    std::map<std::string, std::shared_ptr<MetroStation>> prevStations;

    MetroStation(const std::string& stationName, bool oneWay = false);
};

#endif