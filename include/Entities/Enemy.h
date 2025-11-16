#pragma once
#include "Character.h"
#include <string>
#include <memory>

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

public:
    FallenShadowPaladin(int playerLevel);
    void updateAI(float deltaTime) override;
    void performAttack() override;
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



