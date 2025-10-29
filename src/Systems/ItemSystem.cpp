#include "ItemSystem.h"

std::vector<Item> ItemSystem::itemDatabase;

void ItemSystem::initialize() {
    // Consumables - Potions
    itemDatabase.push_back({ItemType::HEALTH_POTION, "Health Potion", "Restore 50 HP", Color{255, 100, 100, 255}, 1, 10, 50, 0});
    itemDatabase.push_back({ItemType::SPEED_POTION, "Speed Potion", "+50% speed for 8s", Color{0, 200, 255, 255}, 2, 5, 0, 8});
    itemDatabase.push_back({ItemType::STEALTH_POTION, "Stealth Potion", "Invisible for 6s", Color{100, 100, 150, 255}, 2, 5, 0, 6});
    itemDatabase.push_back({ItemType::RAGE_POTION, "Rage Potion", "+30% damage for 10s", Color{200, 0, 0, 255}, 2, 5, 0, 10});
    itemDatabase.push_back({ItemType::MANA_POTION, "Mana Potion", "Restore all spell charges", Color{150, 100, 255, 255}, 2, 5, 0, 0});
    itemDatabase.push_back({ItemType::HOLY_WATER_OF_LIFE, "Holy Water of Life", "Cure all curses", Color{255, 255, 200, 255}, 3, 3, 0, 0});
    itemDatabase.push_back({ItemType::STARDUST, "Stardust", "Enhance mana permanently", Color{200, 200, 255, 255}, 3, 10, 0, 0});

    // Food
    itemDatabase.push_back({ItemType::MEAT, "Raw Meat", "Restore 30 HP", Color{160, 82, 45, 255}, 1, 20, 30, 0});
    itemDatabase.push_back({ItemType::APPLE, "Apple", "Restore 15 HP", Color{200, 0, 0, 255}, 1, 25, 15, 0});
    itemDatabase.push_back({ItemType::BREAD, "Bread", "Restore 20 HP", Color{210, 180, 140, 255}, 1, 30, 20, 0});
    itemDatabase.push_back({ItemType::MAGICAL_FRUIT, "Magical Fruit", "Restore 60 HP + Mana", Color{255, 20, 147, 255}, 3, 3, 60, 0});
    itemDatabase.push_back({ItemType::CHEESE, "Cheese", "Restore 25 HP", Color{255, 215, 0, 255}, 1, 15, 25, 0});

    // Equipment
    itemDatabase.push_back({ItemType::SHIELD_PENDANT, "Shield Pendant", "Create protective barrier", Color{173, 216, 230, 255}, 3, 1, 0, 0});
    itemDatabase.push_back({ItemType::RING_OF_FIRE, "Ring of Fire", "+20% fire damage", Color{255, 69, 0, 255}, 3, 1, 0, 0});
    itemDatabase.push_back({ItemType::AMULET_OF_ICE, "Amulet of Ice", "+20% frost damage", Color{0, 191, 255, 255}, 3, 1, 0, 0});
    itemDatabase.push_back({ItemType::BOOTS_OF_SWIFTNESS, "Boots of Swiftness", "+25% movement speed", Color{64, 224, 208, 255}, 3, 1, 0, 0});
    itemDatabase.push_back({ItemType::CLOAK_OF_INVISIBILITY, "Cloak of Invisibility", "Stealth on demand", Color{128, 128, 128, 255}, 4, 1, 0, 0});
    itemDatabase.push_back({ItemType::PALADIN_NECKLACE, "Paladin Necklace", "Takes 30% less damage", Color{255, 215, 0, 255}, 4, 1, 0, 0});

    // Weapons
    itemDatabase.push_back({ItemType::SCORCHING_GAUNTLET, "Scorching Gauntlet", "+40% fire damage, Lvl 10+", Color{255, 100, 0, 255}, 3, 1, 0, 0});
    itemDatabase.push_back({ItemType::DEMON_KING_LONG_SWORD, "Demon King Long Sword", "Massive damage, Shadow Paladin drop", Color{200, 0, 0, 255}, 4, 1, 0, 0});
    itemDatabase.push_back({ItemType::VENOM_SWORD, "Venom Sword", "Poison damage, Giant Snake drop", Color{0, 200, 0, 255}, 3, 1, 0, 0});

    // Throwables
    itemDatabase.push_back({ItemType::SHURIKEN, "Shuriken", "Throw for 15 damage", Color{192, 192, 192, 255}, 2, 20, 15, 0});
    itemDatabase.push_back({ItemType::THROWING_KNIFE, "Throwing Knife", "Throw for 20 damage", Color{169, 169, 169, 255}, 2, 15, 20, 0});
    itemDatabase.push_back({ItemType::MAGIC_ORB, "Magic Orb", "Throw for 30 magic damage", Color{138, 43, 226, 255}, 3, 10, 30, 0});

    // Currency
    itemDatabase.push_back({ItemType::ESSENCE_STONES, "Essence Stone", "Common currency", Color{100, 255, 100, 255}, 1, 999, 0, 0});
    itemDatabase.push_back({ItemType::ORBS, "Orb", "Rare currency", Color{255, 200, 0, 255}, 4, 99, 0, 0});

    // Pets/Evolution
    itemDatabase.push_back({ItemType::SEEDS_OF_EVOLUTION, "Seed of Evolution", "Create fierce goblin companion", Color{0, 200, 100, 255}, 3, 1, 0, 0});

    // Quest Items
    itemDatabase.push_back({ItemType::ANCIENT_KEY, "Ancient Key", "Opens ancient doors", Color{218, 165, 32, 255}, 4, 1, 0, 0});
    itemDatabase.push_back({ItemType::TREASURE_MAP, "Treasure Map", "Leads to treasure", Color{139, 69, 19, 255}, 3, 1, 0, 0});
    itemDatabase.push_back({ItemType::MYSTICAL_RUNE, "Mystical Rune", "Powerful magical artifact", Color{75, 0, 130, 255}, 4, 1, 0, 0});
}

Item ItemSystem::getItem(ItemType type) {
    for (const auto& item : itemDatabase) {
        if (item.type == type) return item;
    }
    return itemDatabase[0]; // Return first item as fallback
}

std::string ItemSystem::getItemName(ItemType type) {
    return getItem(type).name;
}

Color ItemSystem::getItemColor(ItemType type) {
    return getItem(type).color;
}

bool ItemSystem::isConsumable(ItemType type) {
    return type >= ItemType::HEALTH_POTION && type <= ItemType::MANA_POTION;
}

bool ItemSystem::isEquipment(ItemType type) {
    return type >= ItemType::SHIELD_PENDANT && type <= ItemType::CLOAK_OF_INVISIBILITY;
}

bool ItemSystem::isThrowable(ItemType type) {
    return type >= ItemType::SHURIKEN && type <= ItemType::MAGIC_ORB;
}

bool ItemSystem::isFood(ItemType type) {
    return type >= ItemType::MEAT && type <= ItemType::CHEESE;
}
