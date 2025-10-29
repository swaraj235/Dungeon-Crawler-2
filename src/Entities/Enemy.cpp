#include "Enemy.h"
#include "Config.h"
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <memory>
#include "raymath.h"

// Hit flash state tracking
namespace {
    struct HitFlashState {
        float timeLeft = 0.0f;
    };
    std::unordered_map<Enemy*, HitFlashState> hitFlashMap;
}

Enemy::Enemy(EnemyType type, int hp, int lvl, const std::string& spritePath,
             const std::string& name, float spd, int atk, float aggro, float atkRange)
    : Character(hp, lvl, spritePath, name), enemyType(type), tier(EnemyTier::D),
      speed(spd), attackDamage(atk), attackCooldown(2.5f), lastAttackTime(0),
      aggroRange(aggro), attackRange(atkRange), target(nullptr),
      currentState(AIState::IDLE), hitFlashTime(0) {

    // Assign colors based on enemy type
    switch (type) {
        case EnemyType::GOBLIN: displayColor = GREEN; break;
        case EnemyType::SKELETON: displayColor = Color{200, 200, 200, 255}; break;
        case EnemyType::SLIME: displayColor = Color{0, 255, 100, 255}; break;
        case EnemyType::HOUND: displayColor = Color{255, 182, 193, 255}; break;
        case EnemyType::BAT: displayColor = Color{50, 50, 50, 255}; break;
        case EnemyType::CHIMERA_ANT: displayColor = Color{150, 75, 0, 255}; break;
        case EnemyType::WEREWOLF: displayColor = Color{139, 69, 19, 255}; break;
        case EnemyType::CERBERUS: displayColor = Color{100, 0, 0, 255}; break;
        case EnemyType::GIANT_CENTIPEDE: displayColor = Color{128, 0, 128, 255}; break;
        case EnemyType::GIANT_SNAKE: displayColor = YELLOW; break;
        case EnemyType::STONE_TROLL: displayColor = GRAY; break;
        default: displayColor = DARKGRAY; break;
    }
}

Enemy::~Enemy() {}

void Enemy::update(float deltaTime) {
    if (!isAlive) return;

    lastAttackTime += deltaTime;
    hitFlashTime = std::max(0.0f, hitFlashTime - deltaTime);

    updateAI(deltaTime);
}

void Enemy::draw() {
    if (!isAlive) return;

    Color tintColor = displayColor;
    if (hitFlashTime > 0) {
        tintColor = Color{255, 100, 100, 255}; // Red flash on hit
    }

    if (sprite.id != 0) {
        DrawTexture(sprite, (int)position.x, (int)position.y, tintColor);
    } else {
        DrawRectangle((int)position.x, (int)position.y, 32, 32, tintColor);
    }

    // Health bar
    DrawRectangle((int)position.x, (int)position.y - 10, 32, 3, BLACK);
    float healthPercent = (float)health / maxHealth;
    DrawRectangle((int)position.x, (int)position.y - 10, (int)(32 * healthPercent), 3, RED);

    // Name
    DrawText(name.c_str(), (int)position.x - 10, (int)position.y - 25, 10, WHITE);
}

void Enemy::updateAI(float deltaTime) {
    if (!target || !target->getIsAlive()) return;

    float distanceToTarget = Vector2Distance(position, target->getPosition());

    switch (currentState) {
        case AIState::IDLE:
            if (distanceToTarget <= aggroRange) {
                currentState = AIState::CHASING;
            }
            break;

        case AIState::CHASING:
            if (distanceToTarget <= attackRange && canAttack()) {
                currentState = AIState::ATTACKING;
                performAttack();
            } else if (distanceToTarget > aggroRange * 1.5f) {
                currentState = AIState::IDLE;
            } else {
                moveTowardsTarget(deltaTime);
            }
            break;

        case AIState::ATTACKING:
            if (distanceToTarget > attackRange) {
                currentState = AIState::CHASING;
            } else if (canAttack()) {
                performAttack();
            }
            break;
    }
}

void Enemy::moveTowardsTarget(float deltaTime) {
    if (!target) return;

    Vector2 direction = {target->getPosition().x - position.x, target->getPosition().y - position.y};
    float distance = Vector2Length(direction);

    if (distance > 0) {
        direction = Vector2Normalize(direction);
        position.x += direction.x * speed * deltaTime;
        position.y += direction.y * speed * deltaTime;
    }
}

void Enemy::performAttack() {
    if (lastAttackTime < attackCooldown) return;

    lastAttackTime = 0;

    if (target && target->getIsAlive()) {
        int damage = attackDamage / 4; // Reduced damage
        target->takeDamage(damage);
    }
}

bool Enemy::canAttack() const {
    return lastAttackTime >= attackCooldown;
}

void Enemy::applyKnockback(Vector2 from, float force) {
    Vector2 dir = {position.x - from.x, position.y - from.y};
    float len = Vector2Length(dir);

    if (len > 0.001f) {
        dir = Vector2Normalize(dir);
        position.x += dir.x * force;
        position.y += dir.y * force;
    }
}

void Enemy::flashHit(float duration) {
    hitFlashTime = duration;
}

// TIER D ENEMIES

Goblin::Goblin(int playerLevel)
    : Enemy(EnemyType::GOBLIN, 20 + playerLevel * 2, playerLevel,
            "assets/sprites/goblin.png", "Goblin", 50.0f, 5 + playerLevel/2, 80.0f, 35.0f) {
    tier = EnemyTier::D;
    attackCooldown = 2.5f;
}

void Goblin::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Skeleton::Skeleton(int playerLevel)
    : Enemy(EnemyType::SKELETON, 25 + playerLevel * 3, playerLevel,
            "assets/sprites/skeleton.png", "Skeleton", 40.0f, 6 + playerLevel/2, 90.0f, 35.0f) {
    tier = EnemyTier::D;
    attackCooldown = 3.0f;
}

void Skeleton::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Slime::Slime(int playerLevel)
    : Enemy(EnemyType::SLIME, 18 + playerLevel * 2, playerLevel,
            "assets/sprites/slime.png", "Slime", 40.0f, 4 + playerLevel/2, 70.0f, 30.0f),
      bounceTimer(0) {
    tier = EnemyTier::D;
    attackCooldown = 2.8f;
}

void Slime::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Hound::Hound(int playerLevel)
    : Enemy(EnemyType::HOUND, 22 + playerLevel * 2, playerLevel,
            "assets/sprites/hound.png", "Shadow Hound", 80.0f, 5 + playerLevel/2, 100.0f, 40.0f),
      dashCooldown(8.0f), lastDashTime(0) {
    tier = EnemyTier::D;
    attackCooldown = 2.5f;
}

void Hound::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
    lastDashTime += deltaTime;
}

void Hound::dash() {
    // Dash towards player
}

Bat::Bat(int playerLevel)
    : Enemy(EnemyType::BAT, 16 + playerLevel, playerLevel,
            "assets/sprites/bat.png", "Bat", 100.0f, 4 + playerLevel/2, 60.0f, 30.0f),
      flightHeight(0) {
    tier = EnemyTier::D;
    attackCooldown = 2.2f;
}

void Bat::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
    flightHeight += deltaTime * 3.0f;
    position.y += sin(flightHeight) * 2.0f;
}

// TIER C ENEMIES

ChimeraAnt::ChimeraAnt(int playerLevel)
    : Enemy(EnemyType::CHIMERA_ANT, 35 + playerLevel * 3, playerLevel,
            "assets/sprites/chimera_ant.png", "Chimera Ant", 70.0f, 8 + playerLevel, 120.0f, 40.0f) {
    tier = EnemyTier::C;
    attackCooldown = 3.5f;
}

void ChimeraAnt::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Werewolf::Werewolf(int playerLevel)
    : Enemy(EnemyType::WEREWOLF, 40 + playerLevel * 4, playerLevel,
            "assets/sprites/werewolf.png", "Werewolf", 90.0f, 10 + playerLevel, 130.0f, 45.0f),
      isTransformed(false) {
    tier = EnemyTier::C;
    attackCooldown = 3.8f;
}

void Werewolf::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Cerberus::Cerberus(int playerLevel)
    : Enemy(EnemyType::CERBERUS, 45 + playerLevel * 5, playerLevel,
            "assets/sprites/cerberus.png", "Cerberus", 85.0f, 12 + playerLevel * 2, 140.0f, 50.0f),
      headCount(3) {
    tier = EnemyTier::C;
    attackCooldown = 4.0f;
}

void Cerberus::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

void Cerberus::performAttack() {
    Enemy::performAttack();
}

GiantCentipede::GiantCentipede(int playerLevel)
    : Enemy(EnemyType::GIANT_CENTIPEDE, 30 + playerLevel * 3, playerLevel,
            "assets/sprites/centipede.png", "Giant Centipede", 60.0f, 9 + playerLevel, 120.0f, 50.0f) {
    tier = EnemyTier::C;
    attackCooldown = 3.2f;
}

void GiantCentipede::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

GiantSnake::GiantSnake(int playerLevel)
    : Enemy(EnemyType::GIANT_SNAKE, 38 + playerLevel * 3, playerLevel,
            "assets/sprites/snake.png", "Giant Snake", 75.0f, 11 + playerLevel, 115.0f, 45.0f),
      coilCooldown(5.0f) {
    tier = EnemyTier::C;
    attackCooldown = 3.5f;
}

void GiantSnake::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

StoneTroll::StoneTroll(int playerLevel)
    : Enemy(EnemyType::STONE_TROLL, 60 + playerLevel * 6, playerLevel,
            "assets/sprites/troll.png", "Stone Troll", 50.0f, 14 + playerLevel * 2, 100.0f, 50.0f),
      regenerateCooldown(10.0f) {
    tier = EnemyTier::C;
    attackCooldown = 4.5f;
}

void StoneTroll::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

void StoneTroll::regenerate() {
    if (health < maxHealth) {
        health = std::min(maxHealth, health + 5);
    }
}

// Add Shadow Paladin class:
FallenShadowPaladin::FallenShadowPaladin(int playerLevel)
    : Enemy(EnemyType::FALLEN_SHADOW_PALADIN, 180 + playerLevel * 10, playerLevel,  // STRONGER
            "assets/sprites/shadow_paladin.png", "Fallen Shadow Paladin",
            100.0f, 35 + playerLevel * 3, 150.0f, 60.0f),
      dashCooldown(5.0f), lastDashTime(0), isDashing(false) {
    tier = EnemyTier::A;  // CHANGE to Tier A
    attackCooldown = 3.5f;
    displayColor = Color{100, 50, 150, 255};
}

void FallenShadowPaladin::updateAI(float deltaTime) {
    if (!target || !target->getIsAlive()) return;

    lastDashTime += deltaTime;
    float distanceToTarget = Vector2Distance(position, target->getPosition());

    // Dash ability
    if (lastDashTime >= dashCooldown && distanceToTarget < 200.0f) {
        isDashing = true;
        Vector2 direction = Vector2Normalize({target->getPosition().x - position.x,
                                             target->getPosition().y - position.y});
        position.x += direction.x * 150.0f;
        position.y += direction.y * 150.0f;
        lastDashTime = 0;
        isDashing = false;
    }

    switch (currentState) {
        case AIState::IDLE:
            if (distanceToTarget <= aggroRange) {
                currentState = AIState::CHASING;
            }
            break;

        case AIState::CHASING:
            if (distanceToTarget <= attackRange && canAttack()) {
                currentState = AIState::ATTACKING;
                performAttack();
            } else if (distanceToTarget > aggroRange * 1.5f) {
                currentState = AIState::IDLE;
            } else {
                moveTowardsTarget(deltaTime);
            }
            break;

        case AIState::ATTACKING:
            if (distanceToTarget > attackRange) {
                currentState = AIState::CHASING;
            } else if (canAttack()) {
                performAttack();
            }
            break;
    }
}

void FallenShadowPaladin::performAttack() {
    if (lastAttackTime < attackCooldown) return;
    lastAttackTime = 0;

    if (target && target->getIsAlive()) {
        int damage = attackDamage / 3;  // Strong attack
        target->takeDamage(damage);
    }
}

std::unique_ptr<Enemy> Enemy::create(EnemyType type, int playerLevel) {
    switch (type) {
        case EnemyType::GOBLIN: return std::make_unique<Goblin>(playerLevel);
        case EnemyType::SKELETON: return std::make_unique<Skeleton>(playerLevel);
        case EnemyType::SLIME: return std::make_unique<Slime>(playerLevel);
        case EnemyType::HOUND: return std::make_unique<Hound>(playerLevel);
        case EnemyType::BAT: return std::make_unique<Bat>(playerLevel);
        case EnemyType::CHIMERA_ANT: return std::make_unique<ChimeraAnt>(playerLevel);
        case EnemyType::WEREWOLF: return std::make_unique<Werewolf>(playerLevel);
        case EnemyType::CERBERUS: return std::make_unique<Cerberus>(playerLevel);
        case EnemyType::GIANT_CENTIPEDE: return std::make_unique<GiantCentipede>(playerLevel);
        case EnemyType::GIANT_SNAKE: return std::make_unique<GiantSnake>(playerLevel);
        case EnemyType::STONE_TROLL: return std::make_unique<StoneTroll>(playerLevel);
        case EnemyType::FALLEN_SHADOW_PALADIN: return std::make_unique<FallenShadowPaladin>(playerLevel);
        default: return std::make_unique<Goblin>(playerLevel);
    }
}
