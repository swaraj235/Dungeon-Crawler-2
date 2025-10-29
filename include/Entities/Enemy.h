#pragma once
#include "Character.h"
#include <string>
#include <memory>

enum class EnemyType {
    // Tier D
    GOBLIN, SKELETON, SLIME, HOUND, BAT,
    // Tier C
    CHIMERA_ANT, WEREWOLF, CERBERUS, GIANT_CENTIPEDE, GIANT_SNAKE, STONE_TROLL,
    //Tier B
    SKELETON_GIANT, ICE_ELF, GOBLIN_GIANT, MAGE, LAVA_GOLEM,
    // Tier A
    RED_ORC, BAT_WITCH, FALLEN_SHADOW_PALADIN, HARPY_QUEEN, NECROMANCER_GENERAL,
    //Tier S
    DRAGON, TITAN, SKELETON_KING, GOBLIN_MAMA, FROST_KING, ABYSSAL_HYDRA
};

enum class EnemyTier { D, C, B, A, S };
enum class AIState { IDLE, CHASING, ATTACKING };

class Enemy : public Character {
protected:
    EnemyType enemyType;
    EnemyTier tier;

    float speed;
    int attackDamage;
    float attackCooldown;
    float lastAttackTime;

    float aggroRange;
    float attackRange;

    Character* target;
    AIState currentState;

    // Visual effects
    float hitFlashTime;
    Color displayColor;

public:
    Enemy(EnemyType type, int hp, int lvl, const std::string& spritePath,
          const std::string& name, float spd, int atk, float aggro, float atkRange);
    virtual ~Enemy();

    // Virtual methods
    void update(float deltaTime) override;
    void draw() override;

    // AI
    virtual void updateAI(float deltaTime);
    void moveTowardsTarget(float deltaTime);

    // Combat
    virtual void performAttack();
    bool canAttack() const;
    void applyKnockback(Vector2 from, float force);
    void flashHit(float duration = 0.1f);

    // Getters
    EnemyType getEnemyType() const { return enemyType; }
    EnemyTier getTier() const { return tier; }
    int getAttackDamage() const { return attackDamage; }
    void setTarget(Character* t) { target = t; }

    // Factory methods for each enemy type
    static std::unique_ptr<Enemy> create(EnemyType type, int playerLevel);
};

// Tier D Enemies
class Goblin : public Enemy {
public:
    Goblin(int playerLevel);
    void updateAI(float deltaTime) override;
};

class Skeleton : public Enemy {
public:
    Skeleton(int playerLevel);
    void updateAI(float deltaTime) override;
};

class Slime : public Enemy {
private:
    float bounceTimer;
public:
    Slime(int playerLevel);
    void updateAI(float deltaTime) override;
};

class Hound : public Enemy {
private:
    float dashCooldown;
    float lastDashTime;
public:
    Hound(int playerLevel);
    void updateAI(float deltaTime) override;
    void dash();
};

class Bat : public Enemy {
private:
    float flightHeight;
public:
    Bat(int playerLevel);
    void updateAI(float deltaTime) override;
};

// Tier C Enemies
class ChimeraAnt : public Enemy {
public:
    ChimeraAnt(int playerLevel);
    void updateAI(float deltaTime) override;
};

class Werewolf : public Enemy {
private:
    bool isTransformed;
public:
    Werewolf(int playerLevel);
    void updateAI(float deltaTime) override;
};

class Cerberus : public Enemy {
private:
    int headCount;
public:
    Cerberus(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
};

class GiantCentipede : public Enemy {
public:
    GiantCentipede(int playerLevel);
    void updateAI(float deltaTime) override;
};

class GiantSnake : public Enemy {
private:
    float coilCooldown;
public:
    GiantSnake(int playerLevel);
    void updateAI(float deltaTime) override;
};

class StoneTroll : public Enemy {
private:
    float regenerateCooldown;
public:
    StoneTroll(int playerLevel);
    void updateAI(float deltaTime) override;
    void regenerate();
};

// Tier A Enemies
class FallenShadowPaladin : public Enemy {
private:
    float dashCooldown;
    float lastDashTime;
    bool isDashing;

public:
    FallenShadowPaladin(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
};

class HarpyQueen : public Enemy{};

class Necromancer : public Enemy{};
