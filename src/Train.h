#ifndef TRAIN_H
#define TRAIN_H

#include <string>
#include <chrono>
#include <mutex>
#include <atomic>
#include <random>
#include "MetroStation.h"

class MetroLine;

class Train {
public:
    std::string name;
    std::string destination;

    static constexpr int stop_time = 3;
    static constexpr int travel_time = 3;
    static constexpr int bakmil_wait_time = 5;
    static std::mutex print_mutex;
    static std::atomic<int> train_counter;
    static std::random_device rd;
    static std::mt19937 gen;

    Train(const std::string& trainName, const std::string& dest);

    void run(std::shared_ptr<MetroLine> line, int train_id, int num_cycles);
    void run_purple(std::shared_ptr<MetroLine> line);
    void run_lime(std::shared_ptr<MetroLine> line);

    void move(std::shared_ptr<MetroStation>& station, int station_index, int total_stations,
              std::chrono::steady_clock::time_point start_time, bool isForward);

    void moveToBakmil(std::chrono::steady_clock::time_point start_time,
                      std::shared_ptr<MetroStation> bakmilStation, const std::string& routeType);
};

#endif // TRAIN_H