#include "Train.h"
#include "MetroLine.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <random>
#include <mutex>
#include <chrono>
#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>

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

    std::shared_ptr<MetroStation> current_station_obj = line->routeHeads["initial"];
    int station_index = 0;
    int total_stations = line->getStationCount("initial");
    std::shared_ptr<MetroStation> last_moved_to_station = nullptr;

    while (current_station_obj && station_index < total_stations) {
        last_moved_to_station = current_station_obj;
        std::shared_ptr<MetroStation> next_ptr = nullptr;

        if (current_station_obj->nextStations.count("initial")) {
            next_ptr = current_station_obj->nextStations["initial"];
        }

        move(current_station_obj, station_index, total_stations, start_time, true);
        station_index++;

        current_station_obj = next_ptr;
    }

    current_station_obj = last_moved_to_station;
    if (!current_station_obj || current_station_obj->name != "Nariman Narimanov") {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cerr << "(" << name << ") Warning: Expected to be at Nariman Narimanov after initial run, but was at "
                  << (current_station_obj ? current_station_obj->name : "null") << ". Resetting." << std::endl;
        current_station_obj = line->getStation("Nariman Narimanov");
        if (!current_station_obj) {
            std::lock_guard<std::mutex> guard_err(print_mutex);
            std::cerr << "(" << name << ") Error: Could not find Nariman Narimanov station." << std::endl;
            return;
        }
    }

    int cycles = 0;
    while (cycles < num_cycles && !toBakmilRepair) {
        bool forward = (cycles % 2 == 0);
        destination = forward ? (isGreen ? "Dyarnyagyul" : "Icheri Sheher") : "Azi Aslanov";
        std::string route_key = lineType;
        total_stations = line->getStationCount(route_key);
        int current_path_index = 0;

        while (true) {
            std::shared_ptr<MetroStation> next_station_target = nullptr;
            if (forward) {
                if (current_station_obj && current_station_obj->nextStations.count(route_key)) {
                    next_station_target = current_station_obj->nextStations[route_key];
                }
            } else {
                if (current_station_obj && current_station_obj->prevStations.count(route_key)) {
                    next_station_target = current_station_obj->prevStations[route_key];
                }
            }

            if (!next_station_target) {
                break;
            }

            move(next_station_target, current_path_index++, total_stations, start_time, forward);
            current_station_obj = next_station_target;

            if (forward && current_station_obj->name == break_station) {
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
                current_station_obj = line->getStation("Nariman Narimanov");
                if (!current_station_obj) {
                    std::lock_guard<std::mutex> guard_err(print_mutex);
                    std::cerr << "(" << name << ") Error: Could not find Nariman Narimanov after Bakmil trip." << std::endl;
                    return;
                }
            }
        }
        cycles++;
    }

    destination = "Bakmil";
    bakmilRoute = isGreen ? "bakmil_from_green" : "bakmil_from_red";
    moveToBakmil(start_time, line->getStation("Bakmil"), bakmilRoute);
    std::lock_guard<std::mutex> guard(print_mutex);
    std::cout << name << " has completed its route and returned to Bakmil." << std::endl;
}


void Train::run_purple(std::shared_ptr<MetroLine> line) {
    auto start_time = std::chrono::steady_clock::now();
    std::shared_ptr<MetroStation> current_station_obj = line->routeHeads["initial"];
    int station_index = 0;
    int total_stations = line->getStationCount("initial");
    std::shared_ptr<MetroStation> last_station_visited = nullptr;

    destination = "8 November";

    while (current_station_obj) {
        last_station_visited = current_station_obj;
        std::shared_ptr<MetroStation> next_ptr = nullptr;
        if (current_station_obj && current_station_obj->nextStations.count("initial")) {
            next_ptr = current_station_obj->nextStations["initial"];
        }

        move(current_station_obj, station_index++, total_stations, start_time, true);
        current_station_obj = next_ptr;
    }
    current_station_obj = last_station_visited;
    if (!current_station_obj || current_station_obj->name != "8 November") {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cerr << "(" << name << ") Error: Did not reach '8 November' after initial run." << std::endl;
        return;
    }

    for (int cycles = 0; cycles < 10; ++cycles) {
        std::string route_key = "green";
        total_stations = line->getStationCount(route_key);

        int counter = train_counter.fetch_add(1) + 1;
        std::string turnaround_station_name = (counter % 2 != 0) ? "Avtovagzal" : "Hojasan";
        std::string forward_destination = "8 November";

        destination = turnaround_station_name;
        int current_path_index_back = 0;
        while (current_station_obj && current_station_obj->name != turnaround_station_name) {
            std::shared_ptr<MetroStation> next_station_target = nullptr;
            if(current_station_obj && current_station_obj->prevStations.count(route_key)) {
                next_station_target = current_station_obj->prevStations[route_key];
            }
            if (!next_station_target) break;

            move(next_station_target, current_path_index_back++, total_stations, start_time, false);
            current_station_obj = next_station_target;
        }

        if (!current_station_obj || current_station_obj->name != turnaround_station_name) {
            std::lock_guard<std::mutex> guard(print_mutex);
            std::cerr << "(" << name << ") Error: Did not reach turnaround station '" << turnaround_station_name << "'." << std::endl;
            break;
        }

        destination = forward_destination;
        int current_path_index_fwd = 0;
        while (current_station_obj && current_station_obj->name != forward_destination) {
            std::shared_ptr<MetroStation> next_station_target = nullptr;
            if(current_station_obj && current_station_obj->nextStations.count(route_key)) {
                next_station_target = current_station_obj->nextStations[route_key];
            }
            if (!next_station_target) break;

            move(next_station_target, current_path_index_fwd++, total_stations, start_time, true);
            current_station_obj = next_station_target;
        }
        if (!current_station_obj || current_station_obj->name != forward_destination) {
            std::lock_guard<std::mutex> guard(print_mutex);
            std::cerr << "(" << name << ") Error: Did not reach end station '8 November' on forward run." << std::endl;
            break;
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
    std::shared_ptr<MetroStation> last_station_visited = nullptr;

    destination = "Jafar Jabbarly";

    while (current_station) {
        last_station_visited = current_station;
        std::shared_ptr<MetroStation> next_ptr = nullptr;
        if (current_station && current_station->nextStations.count("initial")) {
            next_ptr = current_station->nextStations["initial"];
        }
        move(current_station, station_index++, total_stations, start_time, true);
        current_station = next_ptr;
    }
    current_station = last_station_visited;
    if (!current_station || current_station->name != "Jafar Jabbarly") {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cerr << "(" << name << ") Warning: Did not reach Jafar Jabbarly after initial run." << std::endl;
        return;
    }

    for (int cycles = 0; cycles < 20; ++cycles) {
        bool forward = (cycles % 2 == 0);
        std::string route_key = "green";
        std::shared_ptr<MetroStation> start_node_for_this_run = nullptr;

        if(!current_station) {
            current_station = line->routeHeads[route_key];
            if(!current_station) return;
        }

        if (forward) {
            start_node_for_this_run = line->routeHeads[route_key];
            destination = "Khatai";
        } else {
            start_node_for_this_run = line->getStation("Khatai");
            destination = "Jafar Jabbarly";
        }

        if (!start_node_for_this_run) { break; }

        std::shared_ptr<MetroStation> station_to_process = start_node_for_this_run;
        station_index = 0;
        total_stations = line->getStationCount(route_key);

        while (station_to_process) {
            last_station_visited = station_to_process;
            std::shared_ptr<MetroStation> next_target = nullptr;
            if (forward) {
                if(station_to_process && station_to_process->nextStations.count(route_key)) {
                    next_target = station_to_process->nextStations[route_key];
                }
            } else {
                if(station_to_process && station_to_process->prevStations.count(route_key)) {
                    next_target = station_to_process->prevStations[route_key];
                }
            }

            if (station_to_process == start_node_for_this_run && !forward){
                if(next_target) {
                    move(next_target, station_index++, total_stations, start_time, forward);
                    station_to_process = nullptr;
                    last_station_visited = next_target;
                } else {
                    station_to_process = nullptr;
                }
            } else if (next_target) {
                move(next_target, station_index++, total_stations, start_time, forward);
                station_to_process = next_target;
            } else {
                station_to_process = nullptr;
            }
        }
        current_station = last_station_visited;
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

    lock.lock();

    if (station->isOneWay) {
        while (station->isOccupiedForward) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            lock.lock();
        }
        station->isOccupiedForward = true;
    } else {
        if (isForward) {
            while (station->isOccupiedForward) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                lock.lock();
            }
            station->isOccupiedForward = true;
        } else {
            while (station->isOccupiedBackward) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                lock.lock();
            }
            station->isOccupiedBackward = true;
        }
    }

    {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << "(" << name << " -> " << destination << ") arrived to " << station->name << std::endl;
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
    if (!bakmilStation || (!bakmilStation->prevStations.count(routeType) && !bakmilStation->nextStations.count(routeType) )) {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cerr << "(" << name << ") Error: Invalid state for moveToBakmil. Bakmil or route '" << routeType << "' not found or incomplete." << std::endl;
        return;
    }

    std::shared_ptr<MetroStation> current_station = nullptr;
    std::shared_ptr<MetroStation> target_node_on_path = nullptr;

    target_node_on_path = bakmilStation->prevStations.count(routeType) ? bakmilStation->prevStations[routeType] : nullptr;

    if (!target_node_on_path) {
        std::lock_guard<std::mutex> guard(print_mutex);
        std::cerr << "(" << name << ") Error: Cannot determine path to Bakmil via route '" << routeType << "'." << std::endl;
        move(bakmilStation, 0, 1, start_time, true);
        return;
    }

    current_station = target_node_on_path;
    std::vector<std::shared_ptr<MetroStation>> path_to_bakmil;
    while(current_station) {
        path_to_bakmil.push_back(current_station);
        if(current_station->prevStations.count(routeType)) {
            current_station = current_station->prevStations[routeType];
        } else {
            break;
        }
    }
    std::reverse(path_to_bakmil.begin(), path_to_bakmil.end());
    path_to_bakmil.push_back(bakmilStation);

    int station_index = 0;
    int total_stations = static_cast<int>(path_to_bakmil.size());

    for(auto& station_on_path : path_to_bakmil)
    {
        if (!station_on_path) break;
        move(station_on_path, station_index++, total_stations, start_time, true);
    }
}