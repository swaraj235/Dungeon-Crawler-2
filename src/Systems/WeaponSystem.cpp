#include "WeaponSystem.h"

std::vector<WeaponStats> WeaponSystem::weapons;

void WeaponSystem::initialize() {
    weapons = {
        {"Wooden Sword", 10, 1.0f, 0.0f, "Basic starter weapon", 1},
        {"Iron Katana", 18, 1.2f, 0.05f, "Fast and sharp", 5},
        {"Steel Dagger", 12, 1.5f, 0.10f, "Fastest weapon with crit bonus", 10},
        {"Shuriken", 15, 0.8f, 0.0f, "Ranged throwing weapon", 15},
        {"Shadow Ball", 22, 0.7f, 0.0f, "Magical attack", 20}
    };
}

WeaponStats WeaponSystem::getWeaponStats(const std::string& weaponName) {
    for (const auto& weapon : weapons) {
        if (weapon.name == weaponName) {
            return weapon;
        }
    }
    return weapons[0]; // Return wooden sword as default
}

std::string WeaponSystem::getWeaponDescription(const std::string& weaponName) {
    for (const auto& weapon : weapons) {
        if (weapon.name == weaponName) {
            return weapon.description;
        }
    }
    return "Unknown weapon";
}

int WeaponSystem::getWeaponDamage(const std::string& weaponName, int basePlayerDamage) {
    WeaponStats stats = getWeaponStats(weaponName);
    return basePlayerDamage + stats.baseDamage;
}
