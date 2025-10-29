#pragma once
#include "Character.h"
#include <vector>
#include <memory>
#include "ItemSystem.h"

enum class WeaponType {
    WOODEN_SWORD,
    IRON_KATANA,
    STEEL_DAGGER,
    SHURIKEN,
};

enum class SpellType {
    NONE,
    FIREBALL,
    FROST_NOVA,
    CHAIN_LIGHTNING,
    WHIRLWIND,
    SHADOW_BALL
};

struct Weapon {
    WeaponType type;
    int baseDamage;
    float speedMultiplier;
    float critChance;
    std::string name;
};

struct Spell {
    SpellType type;
    float cooldown;
    float lastCastTime;
    int manaCost;
    std::string name;
};

struct InventoryItem {
    std::string name;
    int quantity;
};

class Player : public Character {
private:
    float speed;
    int attackDamage;
    float attackCooldown;
    float lastAttackTime;
    float critChance;
    float critMultiplier;

    // Movement
    float speedMultiplier;

    // Combat
    Rectangle attackRange;

    // Equipment
    Weapon currentWeapon;
    std::vector<Spell> spells;

    // Inventory
    std::vector<InventoryItem> inventory;
    int maxInventorySize;
    std::vector<ItemType> equippedItems;
    float shieldDuration;
    bool shieldActive;

    // Buffs
    float speedBuffTime;
    float stealthBuffTime;
    float rageBuffTime;
    bool isStealthed;

public:
    Player();
    ~Player();

    // Name
    std::string playerName;

    // Implemented virtual methods
    void update(float deltaTime) override;
    void draw() override;

    // Combat
    void attack();
    int computeAttackDamage() const;
    bool canAttack() const;
    Rectangle getAttackRange() const { return attackRange; }
    void updateAttackRange();

    void equipItem(ItemType item);
    void unequipItem(ItemType item);
    bool isItemEquipped(ItemType item) const;
    void activateShield();
    void drawShield();

    // Spells
    void castSpell(SpellType type);
    bool canCast(SpellType type) const;
    void gainExperience(int amount);

    // Movement
    void handleInput();

    // Inventory
    void addItem(const std::string& itemName, int quantity = 1);
    bool useItem(const std::string& itemName);
    bool hasItem(const std::string& itemName) const;

    // Buffs
    void applySpeedBuff(float duration);
    void applyStealthBuff(float duration);
    void applyRageBuff(float duration);

    void setSpeedMultiplier(float mult) { speedMultiplier = mult; }
    float getSpeedMultiplier() const { return speedMultiplier; }

    // Getters
    const Weapon& getWeapon() const { return currentWeapon; }
    const std::vector<Spell>& getSpells() const { return spells; }
    const std::vector<InventoryItem>& getInventory() const { return inventory; }
    bool getIsStealthed() const { return isStealthed; }
    float getSpeedBuffTime() const { return speedBuffTime; }
    float getRageBuffTime() const { return rageBuffTime; }
    int getAttackDamage() const { return attackDamage; }
    float getCritChance() const { return critChance; }
    float getCritMultiplier() const { return critMultiplier; }

    void setHealth(int h) { health = std::max(0, std::min(h, maxHealth)); }
    void setExperience(int exp) { experience = exp; }
    void resetPlayer() {
        health = maxHealth;
        experience = 0;
        level = 1;
    }
};
