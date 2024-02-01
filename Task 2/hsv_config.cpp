// Tom Smale 10533488

#include "hsv_config.h"
#include <fstream>
#include <iostream>

HSVConfig loadConfig(const char* filepath)
{
    HSVConfig config{{0, 0, 0, 0, 0, 0}};
    std::ifstream file(filepath);
    if (file.is_open()) {
        int i = 0, item = 0;
        while (file >> item) {
            config.data[i] = item;
            i++;
        }
        file.close();
    } else {
        std::cout << "could not open: " << filepath << "\n";
    }
    return config;
}

void saveConfig(const char* filepath, const HSVConfig& config)
{
    std::ofstream file(filepath);
    if (file.is_open()) {
        for (int i = 0; i < HSV_CONFIG_ITEMS; i++) {
            file << config.data[i] << " ";
        }
        file.close();
    }
    else {
        std::cout << "could not open: " << filepath << "\n";
    }
}
