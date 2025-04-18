#include "Train.h"
#include "MetroLine.h"
#include <iostream>
#include <thread>

std::mutex Train::print_mutex;
std::atomic<int> Train::train_counter(0);
std::random_device Train::rd;
std::mt19937 Train::gen(Train::rd());

Train::Train(const std::string& trainName, const std::string& dest)
        : name(trainName), destination(dest) {}

void Train::run(std::shared_ptr<MetroLine> line, int train_id, int num_cycles) {
    auto start_time = std::chrono::steady_clock::now();
    bool toBakmilRepair = false;
    bool toBakmilTrip = false;
    std::string original_destination = destination;
    bool isGreen = (train_id % 2 != 0);
    std::string lineType = isGreen ? "green" : "red";
    std::string bakmilRoute = isGreen ? "bakmil_from_green" : "bakmil_from_red";
    std::string break_station = isGreen ? "Dyarnyagyul" : "Icheri Sheher";

    auto current_station = line->routeHeads["initial"];
    int station_index = 0;
    int total_stations = line->getStationCount("initial");
    while (current_station && station_index < total_stations) {
        move(current_station, station_index++, total_stations, start_time, true);
        current_station = current_station->nextStations["initial"];
    }

    int cycles = 0;
    while (cycles < num_cycles && !toBakmilRepair) {
        bool forward = (cycles % 2 == 0);
        destination = forward ? (isGreen ? "Dyarnyagyul" : "Icheri Sheher") : "Azi Aslanov";

        current_station = line->routeHeads[lineType];
        station_index = 0;
        total_stations = line->getStationCount(lineType);

        while (current_station) {
            move(current_station, station_index++, total_stations, start_time, forward);
            if (current_station->name == break_station && forward) {
                std::uniform_int_distribution<> dis(1, 100);
                int chance = dis(gen);
                if (chance <= 20) {
                    destination = "Bakmil";
                    std::uniform_int_distribution<> repair(1, 100);
                    if (repair(gen) <= 25) {
                        toBakmilRepair = true;
                        std::lock_guard<std::mutex> guard(print_mutex);
                        std::cout << "(" << name << " -> " << destination << ") Train goes to repair" << std::endl;
                    } else {
                        toBakmilTrip = true;
                        std::lock_guard<std::mutex> guard(print_mutex);
                        std::cout << "(" << name << " -> " << destination << ") Trip to Bakmil" << std::endl;
                    }
                    break;
                }
            }
            current_station = forward ? current_station->nextStations[lineType] : current_station->prevStations[lineType];
        }

        if (toBakmilRepair || toBakmilTrip) {
            moveToBakmil(start_time, line->getStation("Bakmil"), bakmilRoute);
            if (toBakmilRepair) {
                std::lock_guard<std::mutex> guard(print_mutex);
                std::cout << name << " has been sent to repair and removed from service." << std::endl;
                return;
            } else {
                std::lock_guard<std::mutex> guard(print_mutex);
                std::cout << "(" << name << " -> " << destination << ") the driver moves to the rear carriage" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(bakmil_wait_time));
                destination = original_destination;
                toBakmilTrip = false;
            }
        }
        cycles++;
    }

    destination = "Bakmil";
    moveToBakmil(start_time, line->getStation("Bakmil"), bakmilRoute);
    std::lock_guard<std::mutex> guard(print_mutex);
    std::cout << name << " has completed its route and returned to Bakmil." << std::endl;
}

void Train::run_purple(std::shared_ptr<MetroLine> line) {
    auto start_time = std::chrono::steady_clock::now();
    auto current_station = line->routeHeads["initial"];
    int station_index = 0;
    int total_stations = line->getStationCount("initial");

    while (current_station) {
        move(current_station, station_index++, total_stations, start_time, true);
        current_station = current_station->nextStations["initial"];
    }

    for (int cycles = 0; cycles < 10; ++cycles) {
        bool forward = (cycles % 2 == 0);
        current_station = line->routeHeads["green"];
        station_index = 0;
        total_stations = line->getStationCount("green");

        while (current_station) {
            move(current_station, station_index++, total_stations, start_time, forward);
            current_station = forward ? current_station->nextStations["green"] : current_station->prevStations["green"];
        }

        int counter = train_counter.fetch_add(1);
        bool toAvtovagzal = (counter % 2 == 0);
        if (toAvtovagzal) {
            destination = "Avtovagzal";
        } else {
            destination = "Hojasan";
        }
    }

    destination = "Hojasan Depo";
    moveToBakmil(start_time, line->getStation("Hojasan Depo"), "bakmil_from_red");
    std::lock_guard<std::mutex> guard(print_mutex);
    std::cout << name << " has completed its route and returned to Hojasan Depo." << std::endl;
}

void Train::run_lime(std::shared_ptr<MetroLine> line) {
    auto start_time = std::chrono::steady_clock::now();
    auto current_station = line->routeHeads["initial"];
    int station_index = 0;
    int total_stations = line->getStationCount("initial");

    while (current_station) {
        move(current_station, station_index++, total_stations, start_time, true);
        current_station = current_station->nextStations["initial"];
    }

    for (int cycles = 0; cycles < 20; ++cycles) {
        bool forward = (cycles % 2 == 0);
        current_station = line->routeHeads["green"];
        station_index = 0;
        total_stations = line->getStationCount("green");

        while (current_station) {
            move(current_station, station_index++, total_stations, start_time, forward);
            current_station = forward ? current_station->nextStations["green"] : current_station->prevStations["green"];
        }
    }

    std::lock_guard<std::mutex> guard(print_mutex);
    std::cout << name << " has completed its working day." << std::endl;
}

void Train::move(std::shared_ptr<MetroStation>& station, int station_index, int total_stations,
                 std::chrono::steady_clock::time_point start_time, bool isForward) {
    std::timed_mutex* mutex = station->stationMutex.get();
    std::unique_lock<std::timed_mutex> lock(*mutex, std::defer_lock);
    {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << "(" << name << " -> " << destination << ") Attempting to lock " << station->name << std::endl;
    }
    if (!lock.try_lock_for(std::chrono::seconds(10))) {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << "(" << name << " -> " << destination << ") Failed to lock mutex for " << station->name << " after 10s" << std::endl;
        return;
    }

    int wait_count = 0;
    if (station->isOneWay) {
        while (station->isOccupiedForward && wait_count < 100) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            lock.lock();
            wait_count++;
        }
        if (wait_count >= 100) {
            std::lock_guard<std::mutex> guard(print_mutex);
            std::cout << "(" << name << " -> " << destination << ") Timed out waiting for " << station->name << " after " << wait_count * 0.1 << "s" << std::endl;
            return;
        }
        station->isOccupiedForward = true;
    } else {
        if (isForward) {
            while (station->isOccupiedForward && wait_count < 100) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                lock.lock();
                wait_count++;
            }
            if (wait_count >= 100) {
                std::lock_guard<std::mutex> guard(print_mutex);
                std::cout << "(" << name << " -> " << destination << ") Timed out waiting for " << station->name << " after " << wait_count * 0.1 << "s" << std::endl;
                return;
            }
            station->isOccupiedForward = true;
        } else {
            while (station->isOccupiedBackward && wait_count < 100) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                lock.lock();
                wait_count++;
            }
            if (wait_count >= 100) {
                std::lock_guard<std::mutex> guard(print_mutex);
                std::cout << "(" << name << " -> " << destination << ") Timed out waiting for " << station->name << " after " << wait_count * 0.1 << "s" << std::endl;
                return;
            }
            station->isOccupiedBackward = true;
        }
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
    {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << "(" << name << " -> " << destination << ") arrived to " << station->name
                  << " (Station " << station_index + 1 << " of " << total_stations
                  << ", Time: " << elapsed << "s)" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(stop_time));

    {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << "(" << name << " -> " << destination << ") leaves station " << station->name << std::endl;
    }

    if (station->isOneWay) {
        station->isOccupiedForward = false;
    } else {
        if (isForward) station->isOccupiedForward = false;
        else station->isOccupiedBackward = false;
    }
    lock.unlock();

    std::this_thread::sleep_for(std::chrono::seconds(travel_time));
}

void Train::moveToBakmil(std::chrono::steady_clock::time_point start_time,
                         std::shared_ptr<MetroStation> bakmilStation, const std::string& routeType) {
    auto current_station = bakmilStation->prevStations[routeType];
    int station_index = 0;
    int total_stations = bakmilStation ? 2 : 1; // Примерное значение
    while (current_station && current_station->name != bakmilStation->name) {
        move(current_station, station_index++, total_stations, start_time, true);
        current_station = current_station->nextStations[routeType];
    }
    if (current_station) {
        move(current_station, station_index, total_stations, start_time, true);
    }
}