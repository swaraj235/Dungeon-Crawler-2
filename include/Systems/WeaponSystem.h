#pragma once
#include <string>
#include <vector>

struct WeaponStats {
    std::string name;
    int baseDamage;
    float speedMultiplier;  // Attack speed
    float critChance;
    std::string description;
    int requiredLevel;
};

class WeaponSystem {
private:
    static std::vector<WeaponStats> weapons;

public:
    static void initialize();
    static WeaponStats getWeaponStats(const std::string& weaponName);
    static std::string getWeaponDescription(const std::string& weaponName);
    static int getWeaponDamage(const std::string& weaponName, int basePlayerDamage);
};
