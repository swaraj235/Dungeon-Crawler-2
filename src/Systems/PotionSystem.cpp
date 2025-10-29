#include "PotionSystem.h"

std::vector<PotionEffect> PotionSystem::potions;

void PotionSystem::initialize() {
    potions = {
        {PotionType::HEALTH, "Health Potion", "Restores 50 HP", 50, 0, 1},
        {PotionType::SPEED, "Speed Potion", "Increases speed by 50% for 8 seconds", 0, 800.0f, 2},
        {PotionType::STEALTH, "Stealth Potion", "Become invisible for 6 seconds", 0, 600.0f, 2},
        {PotionType::RAGE, "Rage Potion", "Increase damage by 30% for 10 seconds", 0, 10.0f, 2},
        {PotionType::MANA, "Mana Potion", "Restore mana", 0, 0, 3},
        {PotionType::HEALTH, "Holy Water of Life", "Restores complete HP & curses", 500, 0, 1},
    };
}

PotionEffect PotionSystem::getPotionEffect(PotionType type) {
    for (const auto& potion : potions) {
        if (potion.type == type) {
            return potion;
        }
    }
    return potions[0];
}

PotionEffect PotionSystem::getPotionByName(const std::string& name) {
    for (const auto& potion : potions) {
        if (potion.name == name) {
            return potion;
        }
    }
    return potions[0];
}

std::string PotionSystem::getPotionDescription(PotionType type) {
    for (const auto& potion : potions) {
        if (potion.type == type) {
            return potion.description;
        }
    }
    return "Unknown potion";
}
