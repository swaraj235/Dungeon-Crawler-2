#pragma once
#include "raylib.h"
#include <memory>
#include <vector>

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
    Texture2D sprite;
    bool hasSprite;
    bool facingRight = true; // for sprite flip

public:
    Companion(CompanionType t, int lvl);
    ~Companion();

    void update(float deltaTime);
    void draw();
    void attack(class Enemy* enemy);
    void takeDamage(int damage);
    void setHealth(int h) { health = std::max(0, std::min(h, maxHealth)); }
    void followPlayer(Vector2 playerPos, float deltaTime);
    void setPosition(Vector2 pos) { position = pos; }
    void setFacingRight(bool val) { facingRight = val; }
    bool getFacingRight() const { return facingRight; }

    // Getters
    CompanionType getType() const { return type; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getLevel() const { return level; }
    Vector2 getPosition() const { return position; }
    bool getIsAlive() const { return isAlive; }
    float getLastAttackTime() const { return lastAttackTime; }
    float getAttackCooldown() const { return attackCooldown; }
    std::string getName() const;
};

class CompanionSystem {
private:
    std::vector<std::unique_ptr<Companion>> companions;

public:
    CompanionSystem();

    void tameCompanion(CompanionType type, int playerLevel, Vector2 pos = {0,0});
    void updateCompanion(float deltaTime, Vector2 playerPos,
                         std::vector<std::unique_ptr<class Enemy>>& enemies,
                         class MapGenerator* map,
                         class EffectSystem* effectSystem = nullptr);
    void drawCompanion();
    void releaseCompanion(); // clears Shadow Paladin
    void releaseAllCompanions(); // clears all companions

    bool hasActiveCompanion() const { 
        for(auto& c : companions) if (c->getType() == CompanionType::FALLEN_SHADOW_PALADIN) return true; 
        return false; 
    }
    Companion* getCompanion() const { 
        for(auto& c : companions) if (c->getType() == CompanionType::FALLEN_SHADOW_PALADIN) return c.get(); 
        return nullptr; 
    }
    const std::vector<std::unique_ptr<Companion>>& getCompanions() const { return companions; }
};
