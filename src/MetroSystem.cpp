#include "MetroSystem.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <functional>
#include <vector>

MetroSystem::MetroSystem(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << configFile << std::endl;
        return;
    }

    json config;
    file >> config;
    for (const auto& stationData : config["stations"]) {
        int id = stationData["id"];
        std::string name = stationData["name"];
        stationRegistry[id] = std::make_shared<MetroStation>(name);
    }

    for (const auto& lineData : config["lines"]) {
        lines.push_back(std::make_shared<MetroLine>(lineData));
    }

    auto hojasan = lines[1]->getStation("Hojasan");
    if (hojasan) {
        hojasan->isOneWay = true;
    }
}


void MetroSystem::runSimulation(int num_main_green, int num_main_red, int num_purple, int num_lime) {
    std::vector<std::thread> trains;
    int train_counter_main = 0;

    for (int i = 0; i < num_main_green; ++i) {
        train_counter_main++;
        std::string trainName = "Train_G/R " + std::to_string(train_counter_main);
        std::string dest = "Dyarnyagyul";
        Train train(trainName, dest);
        trains.emplace_back(std::bind(&Train::run, train, lines[0], (train_counter_main * 2 - 1), 5));
    }

    for (int i = 0; i < num_main_red; ++i) {
        train_counter_main++;
        std::string trainName = "Train_G/R " + std::to_string(train_counter_main);
        std::string dest = "Icheri Sheher";
        Train train(trainName, dest);
        trains.emplace_back(std::bind(&Train::run, train, lines[0], (train_counter_main * 2), 5));
    }


    for (int i = 0; i < num_purple; ++i) {
        std::string trainName = "Train_Pur " + std::to_string(i + 1);
        Train train(trainName, "Hojasan/Avtovagzal");
        trains.emplace_back(std::bind(&Train::run_purple, train, lines[1]));
    }

    for (int i = 0; i < num_lime; ++i) {
        std::string trainName = "Train_Lime " + std::to_string(i + 1);
        Train train(trainName, "Khatai");
        trains.emplace_back(std::bind(&Train::run_lime, train, lines[2]));
    }

    for (auto& t : trains) {
        if(t.joinable()) {
            t.join();
        }
    }
}