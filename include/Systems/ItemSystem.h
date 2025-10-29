#pragma once
#include "raylib.h"
#include <string>
#include <vector>

enum class ItemType {
    // Consumables - Potions
    HEALTH_POTION,
    SPEED_POTION,
    STEALTH_POTION,
    RAGE_POTION,
    MANA_POTION,
    HOLY_WATER_OF_LIFE,      // NEW - Cure all curses
    STARDUST,                  // NEW - Enhance mana

    // Food
    MEAT, APPLE, BREAD, MAGICAL_FRUIT, CHEESE,

    // Equipment/Magic Items
    SHIELD_PENDANT,
    RING_OF_FIRE,
    AMULET_OF_ICE,
    BOOTS_OF_SWIFTNESS,
    CLOAK_OF_INVISIBILITY,
    PALADIN_NECKLACE,          // NEW - Barrier, less damage

    // Weapons
    SCORCHING_GAUNTLET,        // NEW - Level 10+
    DEMON_KING_LONG_SWORD,     // NEW - After killing Shadow Paladin
    VENOM_SWORD,               // NEW - After killing 10 Giant Snakes

    // Throwables
    SHURIKEN, THROWING_KNIFE, MAGIC_ORB,

    // Currency
    ESSENCE_STONES,            // NEW - Common currency
    ORBS,                       // NEW - Heavy/rare currency

    // Pets/Evolution
    SEEDS_OF_EVOLUTION,        // NEW - Create fierce companion

    // Quest Items
    ANCIENT_KEY,
    TREASURE_MAP,
    MYSTICAL_RUNE
};

struct Item {
    ItemType type;
    std::string name;
    std::string description;
    Color color;
    int rarity; // 1=common, 2=rare, 3=epic, 4=legendary
    int maxStack;
    float effectValue; // healing amount, damage boost, etc.
    float effectDuration; // buff duration in seconds
};

class ItemSystem {
private:
    static std::vector<Item> itemDatabase;

public:
    static void initialize();
    static Item getItem(ItemType type);
    static std::string getItemName(ItemType type);
    static Color getItemColor(ItemType type);
    static bool isConsumable(ItemType type);
    static bool isEquipment(ItemType type);
    static bool isThrowable(ItemType type);
    static bool isFood(ItemType type);
};
