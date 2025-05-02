#include "MetroStation.h"

MetroStation::MetroStation(const std::string& stationName, bool oneWay)
        : name(stationName), stationMutex(std::make_shared<std::timed_mutex>()),
          isOccupiedForward(false), isOccupiedBackward(false), isOneWay(oneWay) {}