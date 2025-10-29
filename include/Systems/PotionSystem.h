#pragma once
#include <string>
#include <vector>

enum class PotionType {
    HEALTH,
    SPEED,
    STEALTH,
    RAGE,
    MANA
};

struct PotionEffect {
    PotionType type;
    std::string name;
    std::string description;
    int effectValue;
    float effectDuration;
    int rarity; // 1 = common, 2 = rare, 3 = epic
};

class PotionSystem {
private:
    static std::vector<PotionEffect> potions;

public:
    static void initialize();
    static PotionEffect getPotionEffect(PotionType type);
    static PotionEffect getPotionByName(const std::string& name);
    static std::string getPotionDescription(PotionType type);
};
