#include "SaveSystem.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <chrono>

std::string SaveSystem::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&time);
    // Remove newline at end
    if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
        timeStr.erase(timeStr.length()-1);
    }
    return timeStr;
}

void SaveData::toJSON(const std::string& filename) const{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open save file: " << filename << std::endl;
        return;
    }

    file << "{\n";
    file << "  \"playerLevel\": " << playerLevel << ",\n";
    file << "  \"playerHealth\": " << playerHealth << ",\n";
    file << "  \"playerMaxHealth\": " << playerMaxHealth << ",\n";
    file << "  \"playerExperience\": " << playerExperience << ",\n";
    file << "  \"score\": " << score << ",\n";
    file << "  \"enemiesKilled\": " << enemiesKilled << ",\n";
    file << "  \"currentFloor\": " << currentFloor << ",\n";
    file << "  \"playTime\": " << playTime << ",\n";
    file << "  \"currentWeapon\": \"" << currentWeapon << "\",\n";
    file << "  \"potions\": {\n";
    file << "    \"health\": " << potions.healthPotions << ",\n";
    file << "    \"speed\": " << potions.speedPotions << ",\n";
    file << "    \"stealth\": " << potions.stealthPotions << ",\n";
    file << "    \"rage\": " << potions.ragePotions << ",\n";
    file << "    \"mana\": " << potions.manaPotions << "\n";
    file << "  },\n";
    file << "  \"totalDamageDealt\": " << totalDamageDealt << ",\n";
    file << "  \"totalDamageTaken\": " << totalDamageTaken << ",\n";
    file << "  \"potionsUsed\": " << potionsUsed << ",\n";
    file << "  \"highestFloor\": " << highestFloor << ",\n";
    file << "  \"lastSaveTime\": \"" << lastSaveTime << "\"\n";
    file << "}\n";

    file.close();
    std::cout << "Game saved to " << filename << std::endl;
}

bool SaveData::fromJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("playerLevel") != std::string::npos) {
            sscanf(line.c_str(), "  \"playerLevel\": %d,", &playerLevel);
        } else if (line.find("playerHealth") != std::string::npos && line.find("Max") == std::string::npos) {
            sscanf(line.c_str(), "  \"playerHealth\": %d,", &playerHealth);
        } else if (line.find("playerMaxHealth") != std::string::npos) {
            sscanf(line.c_str(), "  \"playerMaxHealth\": %d,", &playerMaxHealth);
        } else if (line.find("playerExperience") != std::string::npos) {
            sscanf(line.c_str(), "  \"playerExperience\": %d,", &playerExperience);
        } else if (line.find("score") != std::string::npos && line.find("playTime") == std::string::npos) {
            sscanf(line.c_str(), "  \"score\": %d,", &score);
        } else if (line.find("enemiesKilled") != std::string::npos) {
            sscanf(line.c_str(), "  \"enemiesKilled\": %d,", &enemiesKilled);
        } else if (line.find("currentFloor") != std::string::npos) {
            sscanf(line.c_str(), "  \"currentFloor\": %d,", &currentFloor);
        } else if (line.find("playTime") != std::string::npos) {
            sscanf(line.c_str(), "  \"playTime\": %f,", &playTime);
        } else if (line.find("\"health\"") != std::string::npos) {
            sscanf(line.c_str(), "    \"health\": %d,", &potions.healthPotions);
        } else if (line.find("\"speed\"") != std::string::npos) {
            sscanf(line.c_str(), "    \"speed\": %d,", &potions.speedPotions);
        } else if (line.find("\"stealth\"") != std::string::npos) {
            sscanf(line.c_str(), "    \"stealth\": %d,", &potions.stealthPotions);
        } else if (line.find("\"rage\"") != std::string::npos) {
            sscanf(line.c_str(), "    \"rage\": %d,", &potions.ragePotions);
        } else if (line.find("\"mana\"") != std::string::npos) {
            sscanf(line.c_str(), "    \"mana\": %d", &potions.manaPotions);
        } else if (line.find("totalDamageDealt") != std::string::npos) {
            sscanf(line.c_str(), "  \"totalDamageDealt\": %d,", &totalDamageDealt);
        } else if (line.find("totalDamageTaken") != std::string::npos) {
            sscanf(line.c_str(), "  \"totalDamageTaken\": %d,", &totalDamageTaken);
        } else if (line.find("highestFloor") != std::string::npos) {
            sscanf(line.c_str(), "  \"highestFloor\": %d,", &highestFloor);
        }
    }

    file.close();
    std::cout << "Game loaded from " << filename << std::endl;
    return true;
}

bool SaveSystem::save(const SaveData& data, const std::string& filename) {
    const_cast<SaveData&>(data).toJSON(filename);
    return true;
}

bool SaveSystem::load(SaveData& data, const std::string& filename) {
    return data.fromJSON(filename);
}

bool SaveSystem::saveExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}
