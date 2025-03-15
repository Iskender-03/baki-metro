#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <atomic>

class MetroStation {
public:
    std::string name;
    std::shared_ptr<std::mutex> stationMutex;
    bool isOccupied;

    MetroStation(const std::string& stationName)
        : name(stationName), stationMutex(std::make_shared<std::mutex>()), isOccupied(false) {}
};

class Train {
public:
    std::string name;
    std::string destination; // Теперь destination будет меняться динамически
    static constexpr int stop_time = 1;
    static constexpr int travel_time = 1;
    static std::mutex print_mutex;
    static std::atomic<int> purple_counter;

    Train(const std::string& trainName, const std::string& dest)
        : name(trainName), destination(dest) {}

    void run(std::vector<std::shared_ptr<MetroStation>>& initial_route,
             std::vector<std::shared_ptr<MetroStation>>& cycle_route,
             std::vector<std::shared_ptr<MetroStation>>& final_route,
             int num_cycles) {
        auto start_time = std::chrono::steady_clock::now();

        int initial_stations = initial_route.size();
        for (int i = 0; i < initial_stations; ++i) {
            move(initial_route[i], i, initial_stations, start_time);
        }

        int cycle_stations = cycle_route.size();
        int cycles = 0;

        while (cycles < num_cycles) {
            for (int i = 1; i < cycle_stations; ++i) {
                move(cycle_route[i], i, cycle_stations, start_time);
            }
            for (int i = cycle_stations - 2; i >= 0; --i) {
                move(cycle_route[i], i, cycle_stations - 1, start_time);
            }
            cycles++;
        }

        int final_stations = final_route.size();
        for (int i = 0; i < final_stations; ++i) {
            move(final_route[i], i, final_stations, start_time);
        }

        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << name << " has completed its route." << std::endl;
    }

    void run_purple(std::vector<std::shared_ptr<MetroStation>>& initial_route,
                    std::vector<std::shared_ptr<MetroStation>>& full_cycle_route,
                    std::vector<std::shared_ptr<MetroStation>>& short_cycle_route,
                    std::vector<std::shared_ptr<MetroStation>>& final_to_avtovagzal,
                    std::vector<std::shared_ptr<MetroStation>>& final_to_hojasan) {
        auto start_time = std::chrono::steady_clock::now();

        // Начальный выезд из Hojasan Depo
        int initial_stations = initial_route.size();
        for (int i = 0; i < initial_stations; ++i) {
            move(initial_route[i], i, initial_stations, start_time);
        }

        // 3 цикла с чередованием
        for (int cycles = 0; cycles < 3; ++cycles) {
            // Движение до 8 November
            int full_cycle_stations = full_cycle_route.size();
            for (int i = 1; i < full_cycle_stations; ++i) {
                move(full_cycle_route[i], i, full_cycle_stations, start_time);
            }

            // Чередование маршрутов на 8 November
            int counter = purple_counter.fetch_add(1);
            bool toAvtovagzal = (counter % 2 == 0);

            if (toAvtovagzal) {
                destination = "Avtovagzal"; // Обновляем направление
                // Короткий маршрут до Avtovagzal и обратно
                int short_cycle_stations = short_cycle_route.size();
                for (int i = 1; i < short_cycle_stations; ++i) {
                    move(short_cycle_route[i], i, short_cycle_stations, start_time);
                }
                for (int i = short_cycle_stations - 2; i >= 0; --i) {
                    move(short_cycle_route[i], i, short_cycle_stations - 1, start_time);
                }
            } else {
                destination = "Hojasan"; // Обновляем направление
                // Полный маршрут до Hojasan и обратно
                for (int i = full_cycle_stations - 2; i >= 0; --i) {
                    move(full_cycle_route[i], i, full_cycle_stations - 1, start_time);
                }
            }
        }

        // Финальный маршрут
        int final_stations;
        std::vector<std::shared_ptr<MetroStation>>& final_route = (purple_counter % 2 == 0) ? final_to_avtovagzal : final_to_hojasan;
        final_stations = final_route.size();
        destination = "Hojasan Depo"; // Финальное направление — депо
        for (int i = 0; i < final_stations; ++i) {
            move(final_route[i], i, final_stations, start_time);
        }

        std::lock_guard<std::mutex> guard(print_mutex);
        std::cout << name << " has completed its route and returned to Hojasan Depo." << std::endl;
    }

    void move(std::shared_ptr<MetroStation>& station, int station_index, int total_stations,
              std::chrono::steady_clock::time_point start_time) {
        std::unique_lock<std::mutex> lock(*station->stationMutex);

        while (station->isOccupied) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            lock.lock();
        }

        station->isOccupied = true;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        {
            std::lock_guard<std::mutex> guard(print_mutex);
            std::cout << name << " (to " << destination << ") arrived at " << station->name
                      << " (Station " << station_index + 1 << " of " << total_stations
                      << ", Time: " << elapsed << "s)" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(stop_time));

        {
            std::lock_guard<std::mutex> guard(print_mutex);
            std::cout << name << " (to " << destination << ") leaves station " << station->name << std::endl;
        }

        station->isOccupied = false;
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(travel_time));
    }
};

std::mutex Train::print_mutex;
std::atomic<int> Train::purple_counter(0);

class MetroSystem {
private:
    std::unordered_map<std::string, std::shared_ptr<MetroStation>> stationRegistry;

    std::vector<std::shared_ptr<MetroStation>> createRoute(const std::vector<std::string>& stationNames) {
        std::vector<std::shared_ptr<MetroStation>> route;
        for (const auto& name : stationNames) {
            if (stationRegistry.find(name) == stationRegistry.end()) {
                stationRegistry[name] = std::make_shared<MetroStation>(name);
            }
            route.push_back(stationRegistry[name]);
        }
        return route;
    }

public:
    struct LineRoutes {
        std::vector<std::shared_ptr<MetroStation>> main_route;
        std::vector<std::shared_ptr<MetroStation>> initial_1;
        std::vector<std::shared_ptr<MetroStation>> initial_2;
        std::vector<std::shared_ptr<MetroStation>> final_1;
        std::vector<std::shared_ptr<MetroStation>> final_2;
    };

    LineRoutes green_line;
    LineRoutes red_line;
   /* LineRoutes purple_line;
    std::vector<std::shared_ptr<MetroStation>> purple_short;
    LineRoutes lime_line;*/

    MetroSystem() {
        green_line.main_route = createRoute({
            "Dyarnyagyul", "Azadlyg Prospekt", "Nasimi", "Memar Ajami", "20 Yanvar",
            "Inshaatchilar", "Elyemlyar Akademiyasy", "Nizami", "28 May", "Ganjlik",
            "Nariman Narimanov", "Ulduz", "Keroglu", "Gara Garayev", "Neftchilyar",
            "Khalglar Dostlugu", "Ahmedli", "Azi Aslanov"
        });
        green_line.initial_1 = createRoute({"Bakmil", "Nariman Narimanov", "Ganjlik", "28 May", "Nizami",
                                          "Elyemlyar Akademiyasy", "Inshaatchilar", "20 Yanvar", "Memar Ajami",
                                          "Nasimi", "Azadlyg Prospekt", "Dyarnyagyul"});
        green_line.initial_2 = createRoute({"Bakmil", "Ulduz", "Keroglu", "Gara Garayev", "Neftchilyar",
                                          "Khalglar Dostlugu", "Ahmedli", "Azi Aslanov"});
        green_line.final_1 = createRoute({"Dyarnyagyul", "Azadlyg Prospekt", "Nasimi", "Memar Ajami",
                                        "20 Yanvar", "Inshaatchilar", "Elyemlyar Akademiyasy", "Nizami",
                                        "28 May", "Ganjlik", "Nariman Narimanov", "Bakmil"});
        green_line.final_2 = green_line.final_1;

        red_line.main_route = createRoute({
            "Icheri Sheher", "Sahil", "28 May", "Ganjlik", "Nariman Narimanov",
            "Ulduz", "Keroglu", "Gara Garayev", "Neftchilyar", "Khalglar Dostlugu",
            "Ahmedli", "Azi Aslanov"
        });
        red_line.initial_1 = createRoute({"Bakmil", "Nariman Narimanov", "Ganjlik", "28 May", "Sahil", "Icheri Sheher"});
        red_line.initial_2 = createRoute({"Bakmil", "Ulduz", "Keroglu", "Gara Garayev", "Neftchilyar",
                                        "Khalglar Dostlugu", "Ahmedli", "Azi Aslanov"});
        red_line.final_1 = createRoute({"Icheri Sheher", "Sahil", "28 May", "Ganjlik", "Nariman Narimanov", "Bakmil"});
        red_line.final_2 = red_line.final_1;

        /*purple_line.main_route = createRoute({"Hojasan", "Avtovagzal", "Memar Ajami 2", "8 November"});
        purple_short = createRoute({"8 November", "Memar Ajami 2", "Avtovagzal"});
        purple_line.initial_1 = createRoute({"Hojasan Depo", "Hojasan", "Avtovagzal", "Memar Ajami 2", "8 November"});
        purple_line.final_1 = createRoute({"8 November", "Memar Ajami 2", "Avtovagzal", "Hojasan", "Hojasan Depo"});
        purple_line.final_2 = createRoute({"Avtovagzal", "Hojasan", "Hojasan Depo"});

        lime_line.main_route = createRoute({"Jafar Jabbarly", "Khatai"});
        lime_line.initial_1 = createRoute({"Jafar Jabbarly"});
        lime_line.initial_2 = createRoute({"Khatai"});
        lime_line.final_1 = createRoute({"Jafar Jabbarly"});
        lime_line.final_2 = createRoute({"Khatai"});*/
    }

    void runSimulation() {
        std::vector<std::thread> trains;

        // Зеленая линия: 10 поездов
        for (int i = 1; i <= 5; ++i) {
            std::string trainName = "Green " + std::to_string(i);
            trains.emplace_back(&Train::run, Train(trainName, "Dyarnyagyul"),
                                std::ref(green_line.initial_1), std::ref(green_line.main_route),
                                std::ref(green_line.final_1), 3);
        }
        for (int i = 6; i <= 10; ++i) {
            std::string trainName = "Green " + std::to_string(i);
            trains.emplace_back(&Train::run, Train(trainName, "Azi Aslanov"),
                                std::ref(green_line.initial_2), std::ref(green_line.main_route),
                                std::ref(green_line.final_2), 3);
        }

        // Красная линия: 10 поездов
        for (int i = 1; i <= 5; ++i) {
            std::string trainName = "Red " + std::to_string(i);
            trains.emplace_back(&Train::run, Train(trainName, "Icheri Sheher"),
                                std::ref(red_line.initial_1), std::ref(red_line.main_route),
                                std::ref(red_line.final_1), 3);
        }
        for (int i = 6; i <= 10; ++i) {
            std::string trainName = "Red " + std::to_string(i);
            trains.emplace_back(&Train::run, Train(trainName, "Azi Aslanov"),
                                std::ref(red_line.initial_2), std::ref(red_line.main_route),
                                std::ref(red_line.final_2), 3);
        }

        // Фиолетовая линия: 4 поезда с чередованием и динамическим направлением
        /*trains.emplace_back(&Train::run_purple, Train("Purple 1", "Hojasan/Avtovagzal"),
                            std::ref(purple_line.initial_1), std::ref(purple_line.main_route),
                            std::ref(purple_short), std::ref(purple_line.final_1), std::ref(purple_line.final_2));
        trains.emplace_back(&Train::run_purple, Train("Purple 2", "Hojasan/Avtovagzal"),
                            std::ref(purple_line.initial_1), std::ref(purple_line.main_route),
                            std::ref(purple_short), std::ref(purple_line.final_1), std::ref(purple_line.final_2));
        trains.emplace_back(&Train::run_purple, Train("Purple 3", "Hojasan/Avtovagzal"),
                            std::ref(purple_line.initial_1), std::ref(purple_line.main_route),
                            std::ref(purple_short), std::ref(purple_line.final_1), std::ref(purple_line.final_2));
        trains.emplace_back(&Train::run_purple, Train("Purple 4", "Hojasan/Avtovagzal"),
                            std::ref(purple_line.initial_1), std::ref(purple_line.main_route),
                            std::ref(purple_short), std::ref(purple_line.final_1), std::ref(purple_line.final_2));

        // Салатовая линия: 2 поезда
        trains.emplace_back(&Train::run, Train("Lime 1", "Khatai"),
                            std::ref(lime_line.initial_1), std::ref(lime_line.main_route),
                            std::ref(lime_line.final_1), 10);
        trains.emplace_back(&Train::run, Train("Lime 2", "Jafar Jabbarly"),
                            std::ref(lime_line.initial_2), std::ref(lime_line.main_route),
                            std::ref(lime_line.final_2), 10);*/

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