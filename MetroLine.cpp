#include "MetroLine.h"
#include "Utils.h"

MetroLine::MetroLine(const json& lineData) {
    name = lineData["name"];
    isOneWay = lineData["isOneWay"];
    isPurple = lineData["isPurple"];

    auto buildRoute = [&](const std::string& routeName, const json& routeData) {
        if (!routeData.is_array() || routeData.empty()) return;
        std::shared_ptr<MetroStation> prev = nullptr;
        for (size_t i = 0; i < routeData.size(); ++i) {
            int id = routeData[i];
            if (!stationRegistry.count(id)) {
                throw std::runtime_error("Station ID " + std::to_string(id) + " not found");
            }
            auto current = stationRegistry[id];
            if (i == 0) {
                routeHeads[routeName] = current;
            }
            if (prev) {
                prev->nextStations[routeName] = current;
                current->prevStations[routeName] = prev;
            }
            prev = current;
        }
    };

    buildRoute("initial", lineData["initial"]);
    buildRoute("green", lineData["green"]);
    buildRoute("red", lineData["red"]);
    buildRoute("bakmil_from_green", lineData["bakmil_from_green"]);
    buildRoute("bakmil_from_red", lineData["bakmil_from_red"]);

    startStation = routeHeads["initial"] ? routeHeads["initial"] : routeHeads["green"];
}

std::shared_ptr<MetroStation> MetroLine::getStation(const std::string& name) {
    for (const auto& [route, head] : routeHeads) {
        auto current = head;
        while (current) {
            if (current->name == name) return current;
            current = current->nextStations[route];
        }
    }
    return nullptr;
}

int MetroLine::getStationCount(const std::string& routeType) {
    int count = 0;
    auto current = routeHeads[routeType];
    while (current) {
        ++count;
        current = current->nextStations[routeType];
    }
    return count;
}