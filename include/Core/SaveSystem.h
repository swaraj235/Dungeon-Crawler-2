#pragma once
#include <string>
#include <vector>

// Potion inventory
struct PotionInventory {
    int healthPotions = 0;
    int speedPotions = 0;
    int stealthPotions = 0;
    int ragePotions = 0;
    int manaPotions = 0;
};

// Save data structure (matches JSON format)
struct SaveData {
    // Player stats
    std::string playerName;  // ← ADD THIS
    int playerLevel;
    int playerHealth;
    int playerMaxHealth;
    int playerExperience;

    // Game progress
    int score;
    int enemiesKilled;
    int currentFloor; // Map changes every 5 levels
    float playTime; // In seconds

    // Equipment
    std::string currentWeapon = "Wooden Sword";

    // Inventory
    PotionInventory potions;

    // Statistics
    int totalDamageDealt;
    int totalDamageTaken;
    int potionsUsed;
    int highestFloor;

    // Timestamp
    std::string lastSaveTime;

    // Methods
    void toJSON(const std::string& filename) const;
    bool fromJSON(const std::string& filename);
};

class SaveSystem {
public:
    static bool save(const SaveData& data, const std::string& filename);
    static bool load(SaveData& data, const std::string& filename);
    static bool saveExists(const std::string& filename);
    static std::vector<std::string> getAllSaves();
    static std::string getCurrentTimestamp();  // ← ADD THIS LINE
    static bool deleteSave(const std::string& filename);
};