#pragma once
#include "raylib.h"
#include <memory>

enum class CompanionType {
    NONE,
    FALLEN_SHADOW_PALADIN,
    SEED_OF_EVOLUTION
};

class Companion {
private:
    CompanionType type;
    int health;
    int maxHealth;
    int level;
    Vector2 position;
    bool isAlive;
    float attackCooldown;
    float lastAttackTime;

public:
    Companion(CompanionType t, int lvl);

    void update(float deltaTime);
    void draw();
    void attack(class Enemy* enemy);
    void takeDamage(int damage);
    void followPlayer(Vector2 playerPos);

    // Getters
    CompanionType getType() const { return type; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getLevel() const { return level; }
    Vector2 getPosition() const { return position; }
    bool getIsAlive() const { return isAlive; }
    std::string getName() const;
};

class CompanionSystem {
private:
    std::unique_ptr<Companion> currentCompanion;
    bool hasCompanion;

public:
    CompanionSystem();

    void tameCompanion(CompanionType type, int playerLevel);
    void updateCompanion(float deltaTime);
    void drawCompanion();
    void releaseCompanion();

    bool hasActiveCompanion() const { return hasCompanion && currentCompanion != nullptr; }
    Companion* getCompanion() const { return currentCompanion.get(); }
};
