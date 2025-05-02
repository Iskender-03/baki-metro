#include "MetroSystem.h"
#include <iostream>
#include <limits>
#include <string>

int get_train_count_input(const std::string& line_name) {
    int count = 0;
    while (true) {
        std::cout << line_name << ": ";
        std::cin >> count;
        if (std::cin.good() && count >= 0) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return count;
        } else {
            std::cout << "Error: Please enter a non-negative integer.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

int main() {
    std::cout << "Welcome to the Baku Metro Simulation!" << std::endl;
    std::cout << "Enter the number of trains for each line:" << std::endl;

    int main_green_count = get_train_count_input("Green/Main (->Dyarnyagyul)");
    int main_red_count   = get_train_count_input("Red/Main   (->Icheri Sheher)");
    int purple_count     = get_train_count_input("Purple");
    int lime_count       = get_train_count_input("Lime");

    MetroSystem metro("metro_config.json");

    metro.runSimulation(main_green_count, main_red_count, purple_count, lime_count);

    return 0;
}