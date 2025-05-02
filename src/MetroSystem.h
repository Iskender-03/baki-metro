#ifndef METRO_SYSTEM_H
#define METRO_SYSTEM_H

#include <vector>
#include <memory>
#include <string>
#include "MetroLine.h"
#include "Train.h"

class MetroSystem {
public:
    std::vector<std::shared_ptr<MetroLine>> lines;

    MetroSystem(const std::string& configFile);
    void runSimulation(int num_main_green, int num_main_red, int num_purple, int num_lime);
};

#endif // METRO_SYSTEM_H