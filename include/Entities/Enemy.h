#pragma once
#include "Character.h"
#include <string>
#include <memory>

class Companion;

enum class EnemyType {
    // Tier D
    GOBLIN, SKELETON, SLIME, HOUND, BAT, FIRE_SPIRIT, DARK_SPIRIT, LIGHT_SPIRIT,
    // Tier C
    CHIMERA_ANT, WEREWOLF, CERBERUS, CYCLOPS, MINOTAUR, STONE_GOLEM, SALAMANDER_MAN,HONEY_BEE, SKELETON_HOUND,
    //Tier B
    SKELETON_KNIGHT, ELF_GIRL, GOBLIN_GIANT, MAGE, LAVA_GOLEM, IMP, ANCIENT_MUMMY,
    // Tier A
    RED_ORC, WITCH, FALLEN_SHADOW_PALADIN, HARPY_QUEEN,
    //Tier S
    DRAGON, TITAN, SKELETON_KING, GOBLIN_MAMA, FROST_KING, ABYSSAL_HYDRA, NECROMANCER,
};

enum class EnemyTier { D, C, B, A, S };
enum class AIState { IDLE, WANDERING, CHASING, ATTACKING };

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
    Companion* companionTarget;
    AIState currentState;

    // Direction (for sprite flip)
    bool facingRight;

    // Wander / patrol (IDLE state roaming)
    float wanderTimer;
    Vector2 wanderTarget;

    // Windup animation: counts DOWN before attack lands
    // Game.cpp reads this to fire addEnemyWindup() / addEnemyHitImpact()
    float windupTimer;       // set to attackCooldown * 0.4f on CHASING→attack
    bool  windupFired;       // has the windup effect been fired for this swing?
    bool  justAttacked;      // set true in performAttack(); cleared by Game.cpp after reading

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
    bool getFacingRight() const { return facingRight; }
    EnemyType getType() const { return enemyType; }
    void setTarget(Character* t) { target = t; }
    void setCompanionTarget(Companion* c) { companionTarget = c; }
    Color getDisplayColor() const { return displayColor; }

    // Windup animation state — read by Game.cpp to fire EffectSystem events
    float getWindupTimer() const { return windupTimer; }
    bool  isWindupFired()  const { return windupFired; }
    void  markWindupFired()      { windupFired = true; }
    AIState getAIState()   const { return currentState; }
    bool  getJustAttacked() const { return justAttacked; }
    void  clearJustAttacked()    { justAttacked = false; }

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

class FireHound : public Enemy {
private:
    float dashCooldown;
    float lastDashTime;
public:
    FireHound(int playerLevel);
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

class Fire_Spirit : public Enemy {
public:
    Fire_Spirit(int playerLevel);
    void updateAI(float deltaTime) override;
};

class Dark_Spirit : public Enemy {
public:
    Dark_Spirit(int playerLevel);
    void updateAI(float deltaTime) override;
};

class Light_Spirit : public Enemy {
public:
    Light_Spirit(int playerLevel);
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

class Cyclops : public Enemy {
public:
    Cyclops(int playerLevel);
    void updateAI(float deltaTime) override;
};

class Minotaur : public Enemy {
private:
    float coilCooldown;
public:
    Minotaur(int playerLevel);
    void updateAI(float deltaTime) override;
};

class Stone_Golem : public Enemy {
private:
    float regenerateCooldown;
public:
    Stone_Golem(int playerLevel);
    void updateAI(float deltaTime) override;
    void regenerate();
};

class SalamanderMan : public Enemy {
public:
    SalamanderMan(int playerLevel);
    void updateAI(float deltaTime) override;
};

class HoneyBee : public Enemy {
public:
    HoneyBee(int playerLevel);
    void updateAI(float deltaTime) override;
};

class SkeletonHound : public Enemy {
public:
    SkeletonHound(int playerLevel);
    void updateAI(float deltaTime) override;
};

// Tier B Enemies
class Skeleton_Knight : public Enemy {
public:
    Skeleton_Knight(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
};

class Elf_Girl : public Enemy {
public:
    Elf_Girl(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
};

class Goblin_Giant : public Enemy {
public:
    Goblin_Giant(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
};

class Mage : public Enemy {
public:
    Mage(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
};

class Lava_Golem : public Enemy {
private:
    float regenerateCooldown;

public:
    Lava_Golem(int playerLevel);
    void updateAI(float deltaTime) override;
    void regenerate();
    // void performAttack() override; // optional but consistent
};

class Imp : public Enemy {
private:
    float teleportCooldown;
    float lastTeleportTime;

public:
    Imp(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
};

class AncientMummy : public Enemy {
public:
    AncientMummy(int playerLevel);
    void updateAI(float deltaTime) override;
};

// Tier A Enemies
class FallenShadowPaladin : public Enemy {
private:
    float dashCooldown;
    float lastDashTime;
    bool isDashing;
    bool isBossInstance;
    bool isBroken;

public:
    FallenShadowPaladin(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
    
    void setBossInstance(bool val) { isBossInstance = val; }
    bool getBossInstance() const { return isBossInstance; }
    void setBroken(bool val) { isBroken = val; }
    bool getBroken() const { return isBroken; }
};

class HarpyQueen : public Enemy {
private:
    float swoopCooldown;
    float lastSwoopTime;

public:
    HarpyQueen(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
};

class Witch : public Enemy {
public:
    Witch(int playerLevel);
    void updateAI(float deltaTime) override;
};

// Tier S

class Necromancer : public Enemy {
public:
    Necromancer(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
};



