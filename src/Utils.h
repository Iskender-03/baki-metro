#ifndef UTILS_H
#define UTILS_H

#include <unordered_map>
#include <memory>
#include "MetroStation.h"

extern std::unordered_map<int, std::shared_ptr<MetroStation>> stationRegistry;
std::shared_ptr<MetroStation> getOrCreateStation(int id);

#endif