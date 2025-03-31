#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <atomic>
#include <random>

class MetroStation {
public:
    std::string name;
    std::shared_ptr<std::timed_mutex> stationMutex;
    bool isOccupiedForward;
    bool isOccupiedBackward;
    bool isOneWay;

    MetroStation(const std::string& stationName, bool oneWay = false)
        : name(stationName), stationMutex(std::make_shared<std::timed_mutex>()),
          isOccupiedForward(false), isOccupiedBackward(false), isOneWay(oneWay) {}
};

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

    Train(const std::string& trainName, const std::string& dest)
        : name(trainName), destination(dest) {}

    void run(std::vector<std::shared_ptr<MetroStation>>& initial_route,
             std::vector<std::shared_ptr<MetroStation>>& green_route,
             std::vector<std::shared_ptr<MetroStation>>& red_route,
             std::vector<std::shared_ptr<MetroStation>>& bakmil_from_green,
             std::vector<std::shared_ptr<MetroStation>>& bakmil_from_red,
             int train_id, int num_cycles) {
        auto start_time = std::chrono::steady_clock::now();
        bool toBakmilRepair = false;
        bool toBakmilTrip = false;
        std::string original_destination = destination;
        bool isGreen = (train_id % 2 != 0);

        int initial_stations = initial_route.size();
        for (int i = 0; i < initial_stations; ++i) {
            move(initial_route[i], i, initial_stations, start_time, true);
        }

        int cycles = 0;
        auto& current_route = isGreen ? green_route : red_route;
        std::string break_station = isGreen ? "Dyarnyagyul" : "Icheri Sheher";
        auto& bakmil_route = isGreen ? bakmil_from_green : bakmil_from_red;

        while (cycles < num_cycles && !toBakmilRepair) {
            int route_stations = current_route.size();
            bool forward = (cycles % 2 == 0);
            for (int i = forward ? 1 : route_stations - 2; forward ? i < route_stations : i >= 0; forward ? ++i : --i) {
                move(current_route[i], i, route_stations, start_time, forward);
                if (current_route[i]->name == break_station && forward) {
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
                moveToBakmil(start_time, bakmil_route);
                if (toBakmilRepair) {
                    std::lock_guard<std::mutex> guard(print_mutex);
                    std::cout << name << " has been sent to repair and removed from service." << std::endl;
                    return;
                } else {
                    std::lock_guard<std::mutex> guard(print_mutex);
                    std::cout << "(" << name << " -> " << destination << ") the driver moves to the rear carriage" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(bakmil_wait_time));
                    destination = original_destination;

                    std::vector<std::shared_ptr<MetroStation>> back_from_bakmil;
                    if (isGreen) {
                        back_from_bakmil = {bakmil_route[bakmil_route.size() - 1], bakmil_route[bakmil_route.size() - 2]};
                    } else {
                        back_from_bakmil = {bakmil_route[bakmil_route.size() - 1], bakmil_route[bakmil_route.size() - 2]};
                    }

                    std::unique_lock<std::timed_mutex> bakmil_lock(*back_from_bakmil[0]->stationMutex, std::defer_lock);
                    if (bakmil_lock.try_lock_for(std::chrono::seconds(5))) {
                        back_from_bakmil[0]->isOccupiedForward = false;
                        back_from_bakmil[0]->isOccupiedBackward = false;
                        bakmil_lock.unlock();
                    }

                    std::unique_lock<std::timed_mutex> bakmil_lock_retry(*back_from_bakmil[0]->stationMutex, std::defer_lock);
                    std::unique_lock<std::timed_mutex> nariman_lock(*back_from_bakmil[1]->stationMutex, std::defer_lock);

                    if (bakmil_lock_retry.try_lock_for(std::chrono::seconds(5)) && nariman_lock.try_lock_for(std::chrono::seconds(5))) {
                        auto now = std::chrono::steady_clock::now();
                        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

                        back_from_bakmil[1]->isOccupiedForward = false;
                        back_from_bakmil[1]->isOccupiedBackward = false;

                        std::cout << "(" << name << " -> " << destination << ") Leaving Bakmil to Nariman Narimanov (Time: " << elapsed << "s)" << std::endl;

                        back_from_bakmil[0]->isOccupiedForward = true;
                        std::this_thread::sleep_for(std::chrono::seconds(stop_time));
                        back_from_bakmil[0]->isOccupiedForward = false;

                        std::this_thread::sleep_for(std::chrono::seconds(travel_time));
                        back_from_bakmil[1]->isOccupiedForward = true;

                        now = std::chrono::steady_clock::now();
                        elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
                        std::cout << "(" << name << " -> " << destination << ") Arrived at Nariman Narimanov (Time: " << elapsed << "s)" << std::endl;

                        std::this_thread::sleep_for(std::chrono::seconds(stop_time));
                        back_from_bakmil[1]->isOccupiedForward = false;

                        nariman_lock.unlock();
                        bakmil_lock_retry.unlock();
                    } else {
                        std::cout << "(" << name << " -> " << destination << ") Failed to lock Bakmil or Nariman Narimanov after 5s, skipping departure" << std::endl;
                    }

                    toBakmilTrip = false;
                }
            }
            cycles++;
            destination = (cycles % 2 == 0) ? (isGreen ? "Dyarnyagyul" : "Icheri Sheher") : "Azi Aslanov";
        }

        destination = "Bakmil";
        auto& full_route = isGreen ? green_route : red_route;
        int route_stations = full_route.size();

        int nariman_index = -1;
        for (int i = 0; i < route_stations; ++i) {
            if (full_route[i]->name == "Nariman Narimanov") {
                nariman_index = i;
                break;
            }
        }

        if (nariman_index != -1) {
            bool forward = (cycles % 2 == 0);
            if (forward) {
                for (int i = route_stations - 2; i >= nariman_index; --i) {
                    move(full_route[i], i, route_stations, start_time, false);
                }
            } else {
                for (int i = 1; i <= nariman_index; ++i) {
                    move(full_route[i], i, route_stations, start_time, true);
                }
            }
        }

        std::vector<std::shared_ptr<MetroStation>> final_to_bakmil;
        if (isGreen) {
            final_to_bakmil = {bakmil_from_green[bakmil_from_green.size() - 2], bakmil_from_green[bakmil_from_green.size() - 1]};
        } else {
            final_to_bakmil = {bakmil_from_red[bakmil_from_red.size() - 2], bakmil_from_red[bakmil_from_red.size() - 1]};
        }
        moveToBakmil(start_time, final_to_bakmil);

        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << name << " has completed its route and returned to Bakmil." << std::endl;
    }

    void run_purple(std::vector<std::shared_ptr<MetroStation>>& initial_route,
                    std::vector<std::shared_ptr<MetroStation>>& full_cycle_route,
                    std::vector<std::shared_ptr<MetroStation>>& short_cycle_route,
                    std::vector<std::shared_ptr<MetroStation>>& final_to_avtovagzal,
                    std::vector<std::shared_ptr<MetroStation>>& final_to_hojasan) {
        auto start_time = std::chrono::steady_clock::now();

        int initial_stations = initial_route.size();
        for (int i = 0; i < initial_stations; ++i) {
            move(initial_route[i], i, initial_stations, start_time, true);
        }

        for (int cycles = 0; cycles < 10; ++cycles) {
            int full_cycle_stations = full_cycle_route.size();
            for (int i = 1; i < full_cycle_stations; ++i) {
                move(full_cycle_route[i], i, full_cycle_stations, start_time, true);
            }

            int counter = train_counter.fetch_add(1);
            bool toAvtovagzal = (counter % 2 == 0);
            if (toAvtovagzal) {
                destination = "Avtovagzal";
                int short_cycle_stations = short_cycle_route.size();
                for (int i = 1; i < short_cycle_stations; ++i) {
                    move(short_cycle_route[i], i, short_cycle_stations, start_time, true);
                }
                for (int i = short_cycle_stations - 2; i >= 0; --i) {
                    move(short_cycle_route[i], i, short_cycle_stations - 1, start_time, false);
                }
            } else {
                destination = "Hojasan";
                for (int i = full_cycle_stations - 2; i >= 0; --i) {
                    move(full_cycle_route[i], i, full_cycle_stations - 1, start_time, false);
                }
            }
        }

        destination = "Hojasan Depo";
        int final_stations;
        std::vector<std::shared_ptr<MetroStation>>& final_route = (train_counter % 2 == 0) ? final_to_avtovagzal : final_to_hojasan;
        final_stations = final_route.size();
        for (int i = 0; i < final_stations; ++i) {
            move(final_route[i], i, final_stations, start_time, true);
        }

        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << name << " has completed its route and returned to Hojasan Depo." << std::endl;
    }

    void run_lime(std::vector<std::shared_ptr<MetroStation>>& initial_route,
                  std::vector<std::shared_ptr<MetroStation>>& cycle_route) {
        auto start_time = std::chrono::steady_clock::now();

        int initial_stations = initial_route.size();
        for (int i = 0; i < initial_stations; ++i) {
            move(initial_route[i], i, initial_stations, start_time, true);
        }

        int cycle_stations = cycle_route.size();
        for (int cycles = 0; cycles < 20; ++cycles) {
            for (int i = 1; i < cycle_stations; ++i) {
                move(cycle_route[i], i, cycle_stations, start_time, true);
            }
            for (int i = cycle_stations - 2; i >= 0; --i) {
                move(cycle_route[i], i, cycle_stations - 1, start_time, false);
            }
        }

        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << name << " has completed its working day." << std::endl;
    }

    void move(std::shared_ptr<MetroStation>& station, int station_index, int total_stations,
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

    void moveToBakmil(std::chrono::steady_clock::time_point start_time,
                      std::vector<std::shared_ptr<MetroStation>>& final_route) {
        int final_stations = final_route.size();
        for (int i = 0; i < final_stations; ++i) {
            move(final_route[i], i, final_stations, start_time, true);
        }
    }
};

std::mutex Train::print_mutex;
std::atomic<int> Train::train_counter(0);
std::random_device Train::rd;
std::mt19937 Train::gen(Train::rd());

class MetroLine {
public:
    std::string name;
    std::vector<std::shared_ptr<MetroStation>> initial_route;
    std::vector<std::shared_ptr<MetroStation>> green_route;
    std::vector<std::shared_ptr<MetroStation>> red_route;
    std::vector<std::shared_ptr<MetroStation>> bakmil_from_green;
    std::vector<std::shared_ptr<MetroStation>> bakmil_from_red;
    bool isOneWay;
    bool isPurple;

    MetroLine(const std::string& lineName,
              const std::vector<std::string>& initial,
              const std::vector<std::string>& green,
              const std::vector<std::string>& red,
              const std::vector<std::string>& bakmil_green,
              const std::vector<std::string>& bakmil_red,
              bool oneWay = false, bool purple = false)
        : name(lineName), isOneWay(oneWay), isPurple(purple) {
        initial_route = createRoute(initial, oneWay);
        green_route = createRoute(green, oneWay);
        red_route = createRoute(red, oneWay);
        bakmil_from_green = createRoute(bakmil_green, oneWay);
        bakmil_from_red = createRoute(bakmil_red, oneWay);
    }

private:
    static std::unordered_map<std::string, std::shared_ptr<MetroStation>> stationRegistry;

    std::vector<std::shared_ptr<MetroStation>> createRoute(const std::vector<std::string>& stationNames, bool oneWay) {
        std::vector<std::shared_ptr<MetroStation>> route;
        for (const auto& name : stationNames) {
            if (stationRegistry.find(name) == stationRegistry.end()) {
                bool isOneWayStation = (name == "Hojasan" && stationNames[0] == "Hojasan Depo") || oneWay;
                stationRegistry[name] = std::make_shared<MetroStation>(name, isOneWayStation);
            }
            route.push_back(stationRegistry[name]);
        }
        return route;
    }
};

std::unordered_map<std::string, std::shared_ptr<MetroStation>> MetroLine::stationRegistry;

class MetroSystem {
public:
    std::vector<MetroLine> lines;

    MetroSystem() {
        lines.emplace_back("Main",
                           std::vector<std::string>{"Bakmil", "Nariman Narimanov"},
                           std::vector<std::string>{"Azi Aslanov", "Ahmedli", "Khalglar Dostlugu", "Neftchilyar", "Gara Garayev", 
                                                    "Keroglu", "Ulduz", "Nariman Narimanov", "Ganjlik", "28 May", "Nizami", 
                                                    "Elyemlyar Akademiyasy", "Inshaatchilar", "20 Yanvar", "Memar Ajami", 
                                                    "Nasimi", "Azadlyg Prospekt", "Dyarnyagyul"},
                           std::vector<std::string>{"Azi Aslanov", "Ahmedli", "Khalglar Dostlugu", "Neftchilyar", "Gara Garayev", 
                                                    "Keroglu", "Ulduz", "Nariman Narimanov", "Ganjlik", "28 May", "Sahil", 
                                                    "Icheri Sheher"},
                           std::vector<std::string>{"Dyarnyagyul", "Azadlyg Prospekt", "Nasimi", "Memar Ajami", "20 Yanvar", 
                                                    "Inshaatchilar", "Elyemlyar Akademiyasy", "Nizami", "28 May", "Ganjlik", 
                                                    "Nariman Narimanov", "Bakmil"},
                           std::vector<std::string>{"Icheri Sheher", "Sahil", "28 May", "Ganjlik", "Nariman Narimanov", "Bakmil"},
                           false, false);

        lines.emplace_back("Purple",
                           std::vector<std::string>{"Hojasan Depo", "Hojasan", "Avtovagzal", "Memar Ajami 2", "8 November"},
                           std::vector<std::string>{"Hojasan", "Avtovagzal", "Memar Ajami 2", "8 November"},
                           std::vector<std::string>{},
                           std::vector<std::string>{},
                           std::vector<std::string>{"Avtovagzal", "Hojasan", "Hojasan Depo"},
                           false, true);

        lines.emplace_back("Lime",
                           std::vector<std::string>{"Jafar Jabbarly"},
                           std::vector<std::string>{"Jafar Jabbarly", "Khatai"},
                           std::vector<std::string>{},
                           std::vector<std::string>{},
                           std::vector<std::string>{},
                           true, false);
    }

    void runSimulation() {
        std::vector<std::thread> trains;

        for (int i = 0; i < 6; ++i) {
            std::string trainName = "Train_G/R " + std::to_string(i + 1);
            std::string dest = (i % 2 == 0) ? "Dyarnyagyul" : "Icheri Sheher";
            trains.emplace_back(&Train::run, Train(trainName, dest),
                                std::ref(lines[0].initial_route), std::ref(lines[0].green_route),
                                std::ref(lines[0].red_route), std::ref(lines[0].bakmil_from_green),
                                std::ref(lines[0].bakmil_from_red), i + 1, 5);
        }

        for (int i = 0; i < 4; ++i) {
            std::string trainName = "Train_Pur " + std::to_string(i + 1);
            trains.emplace_back(&Train::run_purple, Train(trainName, "Hojasan/Avtovagzal"),
                                std::ref(lines[1].initial_route), std::ref(lines[1].green_route),
                                std::ref(lines[1].red_route), std::ref(lines[1].bakmil_from_red),
                                std::ref(lines[1].bakmil_from_red));
        }

        for (int i = 0; i < 2; ++i) {
            std::string trainName = "Train_Lime " + std::to_string(i + 1);
            trains.emplace_back(&Train::run_lime, Train(trainName, "Khatai"),
                                std::ref(lines[2].initial_route), std::ref(lines[2].green_route));
        }

        for (auto& t : trains) {
            t.join();
        }
    }
};

int main() {
    MetroSystem metro;
    metro.runSimulation();
    return 0;
}
