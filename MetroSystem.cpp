#include "MetroSystem.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <functional>

MetroSystem::MetroSystem(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть " << configFile << std::endl;
        return;
    }

    json config;

    file >> config;
        for (const auto& station : config["stations"]) {
        int id = station["id"];
        std::string name = station["name"];
        stationRegistry[id] = std::make_shared<MetroStation>(name);
    }

    for (const auto& line : config["lines"]) {
        lines.push_back(std::make_shared<MetroLine>(line));
    }

    auto hojasan = lines[1]->getStation("Hojasan");
    if (hojasan) {
        hojasan->isOneWay = true;
    }
}

void MetroSystem::runSimulation() {
    std::vector<std::thread> trains;

    for (int i = 0; i < 6; ++i) {
        std::string trainName = "Train_G/R " + std::to_string(i + 1);
        std::string dest = (i % 2 == 0) ? "Dyarnyagyul" : "Icheri Sheher";
        Train train(trainName, dest);
        trains.emplace_back(std::bind(&Train::run, train, lines[0], i + 1, 5));
    }

    for (int i = 0; i < 4; ++i) {
        std::string trainName = "Train_Pur " + std::to_string(i + 1);
        Train train(trainName, "Hojasan/Avtovagzal");
        trains.emplace_back(std::bind(&Train::run_purple, train, lines[1]));
    }

    for (int i = 0; i < 2; ++i) {
        std::string trainName = "Train_Lime " + std::to_string(i + 1);
        Train train(trainName, "Khatai");
        trains.emplace_back(std::bind(&Train::run_lime, train, lines[2]));
    }

    for (auto& t : trains) {
        t.join();
    }
}