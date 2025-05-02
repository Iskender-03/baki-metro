#include "Utils.h"
#include <stdexcept>

std::unordered_map<int, std::shared_ptr<MetroStation>> stationRegistry;

std::shared_ptr<MetroStation> getOrCreateStation(int id) {
    if (stationRegistry.count(id)) {
        return stationRegistry[id];
    }
    throw std::runtime_error("Station with ID " + std::to_string(id) + " not found");
}