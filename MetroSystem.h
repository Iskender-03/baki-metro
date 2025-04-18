#ifndef METRO_SYSTEM_H
#define METRO_SYSTEM_H

#include <vector>
#include <memory>
#include "MetroLine.h"
#include "Train.h"

class MetroSystem {
public:
    std::vector<std::shared_ptr<MetroLine>> lines;

    MetroSystem(const std::string& configFile);
    void runSimulation();
};

#endif