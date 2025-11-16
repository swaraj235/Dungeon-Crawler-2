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

        // Tier D
        case EnemyType::GOBLIN: displayColor = GREEN; break;
        case EnemyType::SKELETON: displayColor = Color{200,200,200,255}; break;
        case EnemyType::SLIME: displayColor = Color{0,255,100,255}; break;
        case EnemyType::HOUND: displayColor = Color{255,182,193,255}; break;
        case EnemyType::BAT: displayColor = Color{50,50,50,255}; break;
        case EnemyType::FIRE_SPIRIT: displayColor = ORANGE; break;
        case EnemyType::DARK_SPIRIT: displayColor = DARKPURPLE; break;
        case EnemyType::LIGHT_SPIRIT: displayColor = Color{200,200,50,255}; break;

        // Tier C
        case EnemyType::CHIMERA_ANT: displayColor = Color{150,75,0,255}; break;
        case EnemyType::WEREWOLF: displayColor = Color{139,69,19,255}; break;
        case EnemyType::CERBERUS: displayColor = Color{100,0,0,255}; break;
        case EnemyType::CYCLOPS: displayColor = Color{128,0,128,255}; break;
        case EnemyType::MINOTAUR: displayColor = YELLOW; break;
        case EnemyType::STONE_GOLEM: displayColor = GRAY; break;
        case EnemyType::SALAMANDER_MAN: displayColor = RED; break;
        case EnemyType::HONEY_BEE: displayColor = YELLOW; break;
        case EnemyType::SKELETON_HOUND: displayColor = Color{180,180,180,255}; break;

        // Tier B
        case EnemyType::SKELETON_KNIGHT: displayColor = Color{180,180,255,255}; break;
        case EnemyType::ELF_GIRL: displayColor = Color{150,255,150,255}; break;
        case EnemyType::GOBLIN_GIANT: displayColor = Color{100,200,100,255}; break;
        case EnemyType::MAGE: displayColor = Color{200,50,200,255}; break;
        case EnemyType::LAVA_GOLEM: displayColor = Color{255,80,30,255}; break;
        case EnemyType::IMP: displayColor = Color{255,50,50,255}; break;
        case EnemyType::ANCIENT_MUMMY: displayColor = Color{200,180,100,255}; break;

        // Tier A
        case EnemyType::RED_ORC: displayColor = Color{150,0,0,255}; break;
        case EnemyType::WITCH: displayColor = PURPLE; break;
        case EnemyType::FALLEN_SHADOW_PALADIN: displayColor = Color{100,50,150,255}; break;
        case EnemyType::HARPY_QUEEN: displayColor = Color{255,200,100,255}; break;

        // Tier S
        case EnemyType::NECROMANCER: displayColor = DARKPURPLE; break;

        default:
            displayColor = DARKGRAY;
            break;
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
        DrawTexture(sprite, static_cast<int>(position.x), static_cast<int>(position.y), tintColor);
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
            "assets/sprite/goblin.png", "Goblin", 50.0f, 5 + playerLevel/2, 80.0f, 35.0f) {
    tier = EnemyTier::D;
    attackCooldown = 2.5f;
}

void Goblin::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Skeleton::Skeleton(int playerLevel)
    : Enemy(EnemyType::SKELETON, 25 + playerLevel * 3, playerLevel,
            "assets/sprite/skeleton.png", "Skeleton", 40.0f, 6 + playerLevel/2, 90.0f, 35.0f) {
    tier = EnemyTier::D;
    attackCooldown = 3.0f;
}

void Skeleton::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Slime::Slime(int playerLevel)
    : Enemy(EnemyType::SLIME, 18 + playerLevel * 2, playerLevel,
            "assets/sprite/slime.png", "Slime", 40.0f, 4 + playerLevel/2, 70.0f, 30.0f),
      bounceTimer(0) {
    tier = EnemyTier::D;
    attackCooldown = 2.8f;
}

void Slime::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

FireHound::FireHound(int playerLevel)
    : Enemy(EnemyType::HOUND, 22 + playerLevel * 2, playerLevel,
            "assets/sprite/hound.png", "Fire Hound", 80.0f, 5 + playerLevel/2, 100.0f, 40.0f),
      dashCooldown(8.0f), lastDashTime(0) {
    tier = EnemyTier::D;
    attackCooldown = 2.5f;
}

void FireHound::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
    lastDashTime += deltaTime;
}

void FireHound::dash() {
    // Dash towards player
}

Bat::Bat(int playerLevel)
    : Enemy(EnemyType::BAT, 16 + playerLevel, playerLevel,
            "assets/sprite/bat.png", "Bat", 100.0f, 4 + playerLevel/2, 60.0f, 30.0f),
      flightHeight(0) {
    tier = EnemyTier::D;
    attackCooldown = 2.2f;
}

void Bat::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
    flightHeight += deltaTime * 3.0f;
    position.y += sin(flightHeight) * 2.0f;
}

Fire_Spirit::Fire_Spirit(int playerLevel)
    : Enemy(EnemyType::FIRE_SPIRIT, 20 + playerLevel * 2, playerLevel,
            "assets/sprite/fire_spirit.png", "Fire Spirit",
            80.0f, 5 + playerLevel, 90.0f, 30.0f) {
    tier = EnemyTier::D;
    attackCooldown = 2.2f;
}

void Fire_Spirit::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Dark_Spirit::Dark_Spirit(int playerLevel)
    : Enemy(EnemyType::DARK_SPIRIT, 25 + playerLevel * 2, playerLevel,
            "assets/sprite/dark_spirit.png", "Dark Spirit",
            70.0f, 6 + playerLevel, 100.0f, 35.0f) {
    tier = EnemyTier::D;
    attackCooldown = 2.4f;
}

void Dark_Spirit::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Light_Spirit::Light_Spirit(int playerLevel)
    : Enemy(EnemyType::LIGHT_SPIRIT, 22 + playerLevel * 2, playerLevel,
            "assets/sprite/light_spirit.png", "Light Spirit",
            90.0f, 4 + playerLevel, 80.0f, 30.0f) {
    tier = EnemyTier::D;
    attackCooldown = 2.0f;
}

void Light_Spirit::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

// TIER C ENEMIES

ChimeraAnt::ChimeraAnt(int playerLevel)
    : Enemy(EnemyType::CHIMERA_ANT, 35 + playerLevel * 3, playerLevel,
            "assets/sprite/chimera_ant.png", "Chimera Ant", 70.0f, 8 + playerLevel, 120.0f, 40.0f) {
    tier = EnemyTier::C;
    attackCooldown = 3.5f;
}

void ChimeraAnt::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Werewolf::Werewolf(int playerLevel)
    : Enemy(EnemyType::WEREWOLF, 40 + playerLevel * 4, playerLevel,
            "assets/sprite/werewolf.png", "Werewolf", 90.0f, 10 + playerLevel, 130.0f, 45.0f),
      isTransformed(false) {
    tier = EnemyTier::C;
    attackCooldown = 3.8f;
}

void Werewolf::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Cerberus::Cerberus(int playerLevel)
    : Enemy(EnemyType::CERBERUS, 45 + playerLevel * 5, playerLevel,
            "assets/sprite/cerberus.png", "Cerberus", 85.0f, 12 + playerLevel * 2, 140.0f, 50.0f),
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

Cyclops::Cyclops(int playerLevel)
    : Enemy(EnemyType::CYCLOPS, 30 + playerLevel * 3, playerLevel,
            "assets/sprite/cyclops.png", "Giant Centipede", 60.0f, 9 + playerLevel, 120.0f, 50.0f) {
    tier = EnemyTier::C;
    attackCooldown = 3.2f;
}

void Cyclops::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Minotaur::Minotaur(int playerLevel)
    : Enemy(EnemyType::MINOTAUR, 38 + playerLevel * 3, playerLevel,
            "assets/sprite/minotaur.png", "Minotaur", 75.0f, 11 + playerLevel, 115.0f, 45.0f),
      coilCooldown(5.0f) {
    tier = EnemyTier::C;
    attackCooldown = 3.5f;
}

void Minotaur::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Stone_Golem::Stone_Golem(int playerLevel)
    : Enemy(EnemyType::STONE_GOLEM, 60 + playerLevel * 6, playerLevel,
            "assets/sprite/stone_golem.png", "Stone Golem", 50.0f, 14 + playerLevel * 2, 100.0f, 50.0f),
      regenerateCooldown(10.0f) {
    tier = EnemyTier::C;
    attackCooldown = 4.5f;
}

void Stone_Golem::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

void Stone_Golem::regenerate() {
    if (health < maxHealth) {
        health = std::min(maxHealth, health + 5);
    }
}

SalamanderMan::SalamanderMan(int playerLevel)
    : Enemy(EnemyType::SALAMANDER_MAN, 40 + playerLevel * 3, playerLevel,
            "assets/sprite/salamander.png", "Salamander Man",
            60.0f, 8 + playerLevel * 2, 120.0f, 40.0f) {
    tier = EnemyTier::C;
    attackCooldown = 3.0f;
}

void SalamanderMan::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

HoneyBee::HoneyBee(int playerLevel)
    : Enemy(EnemyType::HONEY_BEE, 18 + playerLevel, playerLevel,
            "assets/sprite/honey_bee.png", "Honey Bee",
            150.0f, 4 + playerLevel, 100.0f, 30.0f) {
    tier = EnemyTier::C;
    attackCooldown = 1.6f;
}

void HoneyBee::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

SkeletonHound::SkeletonHound(int playerLevel)
    : Enemy(EnemyType::SKELETON_HOUND, 35 + playerLevel * 3, playerLevel,
            "assets/sprite/skeleton_hound.png", "Skeleton Hound",
            100.0f, 7 + playerLevel, 120.0f, 45.0f) {
    tier = EnemyTier::C;
    attackCooldown = 2.0f;
}

void SkeletonHound::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

// Tier B Enemies

AncientMummy::AncientMummy(int playerLevel)
    : Enemy(EnemyType::ANCIENT_MUMMY, 80 + playerLevel * 6, playerLevel,
            "assets/sprite/ancient_mummy.png", "Ancient Mummy",
            40.0f, 12 + playerLevel * 2, 140.0f, 50.0f) {
    tier = EnemyTier::B;
    attackCooldown = 3.5f;
}

void AncientMummy::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

Skeleton_Knight::Skeleton_Knight(int playerLevel)
    : Enemy(EnemyType::SKELETON_KNIGHT,
            55 + playerLevel * 5,   // strong HP
            playerLevel,
            "assets/sprite/skeleton_knight.png",
            "Skeleton Knight",
            55.0f,                  // slower
            12 + playerLevel * 2,   // heavy strike
            130.0f,                 // long aggro range
            50.0f)                  // attack range
{
    tier = EnemyTier::B;
    attackCooldown = 3.2f;
    displayColor = Color{180, 180, 255, 255};
}

void Skeleton_Knight::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

void Skeleton_Knight::performAttack() {
    if (lastAttackTime < attackCooldown) return;
    lastAttackTime = 0;

    if (target && target->getIsAlive()) {
        // Knight hits harder
        int damage = attackDamage;
        target->takeDamage(damage);
    }
}

Elf_Girl::Elf_Girl(int playerLevel)
    : Enemy(EnemyType::ELF_GIRL,
            40 + playerLevel * 3,
            playerLevel,
            "assets/sprite/elf_girl.png",
            "Elven Archer",
            100.0f,                 // fast movement
            10 + playerLevel,       // ranged damage
            150.0f,                 // high aggro
            120.0f)                 // long-range attack
{
    tier = EnemyTier::B;
    attackCooldown = 2.0f;
    displayColor = Color{150, 255, 150, 255};
}

void Elf_Girl::updateAI(float deltaTime) {
    // Maintain distance from player
    if (!target) return;

    float dist = Vector2Distance(position, target->getPosition());

    if (dist < attackRange - 40.0f) {
        // Try to kite backwards
        Vector2 dir = Vector2Normalize({position.x - target->getPosition().x,
                                        position.y - target->getPosition().y});
        position.x += dir.x * speed * deltaTime;
        position.y += dir.y * speed * deltaTime;
    }

    Enemy::updateAI(deltaTime);
}

void Elf_Girl::performAttack() {
    if (lastAttackTime < attackCooldown) return;
    lastAttackTime = 0;

    if (target && target->getIsAlive()) {
        int damage = attackDamage;
        target->takeDamage(damage);
    }
}

Lava_Golem::Lava_Golem(int playerLevel)
    : Enemy(EnemyType::LAVA_GOLEM,
            100 + playerLevel * 10,
            playerLevel,
            "assets/sprite/lava_golem.png",
            "Lava Golem",
            35.0f,                    // very slow
            20 + playerLevel * 2,     // heavy damage
            130.0f,
            50.0f),
      regenerateCooldown(8.0f)
{
    tier = EnemyTier::B;
    attackCooldown = 4.5f;
    displayColor = Color{255, 80, 30, 255};
}

void Lava_Golem::updateAI(float deltaTime) {
    regenerateCooldown -= deltaTime;
    if (regenerateCooldown <= 0.0f) {
        regenerate();
        regenerateCooldown = 8.0f;
    }

    Enemy::updateAI(deltaTime);
}

void Lava_Golem::regenerate() {
    health = std::min(maxHealth, health + 10);
}

Mage::Mage(int playerLevel)
    : Enemy(EnemyType::MAGE,
            45 + playerLevel * 4,
            playerLevel,
            "assets/sprite/mage.png",
            "Dark Mage",
            45.0f,                   // slow
            18 + playerLevel * 2,    // magic damage
            150.0f,
            120.0f)                  // long range
{
    tier = EnemyTier::B;
    attackCooldown = 2.8f;
    displayColor = Color{200, 50, 200, 255};
}

void Mage::updateAI(float deltaTime) {
    // Keep distance, attempt to stay at mid-range
    if (!target) return;

    float dist = Vector2Distance(position, target->getPosition());

    if (dist < attackRange - 70.0f) {
        // Move backward to maintain distance
        Vector2 dir = Vector2Normalize({position.x - target->getPosition().x,
                                        position.y - target->getPosition().y});
        position.x += dir.x * speed * deltaTime;
        position.y += dir.y * speed * deltaTime;
    }

    Enemy::updateAI(deltaTime);
}

void Mage::performAttack() {
    if (lastAttackTime < attackCooldown) return;
    lastAttackTime = 0;

    if (target && target->getIsAlive()) {
        // Higher burst damage
        int damage = attackDamage + 5;
        target->takeDamage(damage);
    }
}

Goblin_Giant::Goblin_Giant(int playerLevel)
    : Enemy(EnemyType::GOBLIN_GIANT,
            90 + playerLevel * 8,    // very high HP
            playerLevel,
            "assets/sprite/goblin_giant.png",
            "Goblin Giant",
            40.0f,                   // slow
            15 + playerLevel * 2,    // huge damage
            140.0f,
            55.0f)
{
    tier = EnemyTier::B;
    attackCooldown = 4.0f;
    displayColor = Color{100, 200, 100, 255};
}

void Goblin_Giant::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

void Goblin_Giant::performAttack() {
    if (lastAttackTime < attackCooldown) return;
    lastAttackTime = 0;

    if (target && target->getIsAlive()) {
        int damage = attackDamage;
        target->takeDamage(damage);

        // Apply knockback effect
        // target->applyKnockback(position, 20.0f);
    }
}

Imp::Imp(int playerLevel)
    : Enemy(EnemyType::IMP,
            35 + playerLevel * 2,               // low HP
            playerLevel,
            "assets/sprite/imp.png",
            "Imp",
            120.0f,                             // extremely fast
            6 + playerLevel,                    // small damage
            100.0f,                             // small aggro range
            30.0f),                             // melee range
      teleportCooldown(5.0f),
      lastTeleportTime(0.0f)
{
    tier = EnemyTier::B;
    attackCooldown = 1.8f;                      // fast attacks
    displayColor = Color{255, 50, 50, 255};     // red-ish demonic glow
}

void Imp::updateAI(float deltaTime) {
    if (!target) {
        Enemy::updateAI(deltaTime);
        return;
    }

    lastTeleportTime += deltaTime;

    float distance = Vector2Distance(position, target->getPosition());

    // Imp teleports behind player
    if (lastTeleportTime >= teleportCooldown && distance < 200.0f) {
        Vector2 targetPos = target->getPosition();

        // Teleport behind the player by 30 units
        Vector2 dir = Vector2Normalize({targetPos.x - position.x,
                                        targetPos.y - position.y});
        // Reverse direction to get behind
        position.x = targetPos.x - dir.x * 30.0f;
        position.y = targetPos.y - dir.y * 30.0f;

        lastTeleportTime = 0.0f;

        // After teleport attempt instant attack if in range
        if (Vector2Distance(position, targetPos) <= attackRange) {
            performAttack();
        }

        return;
    }

    // Standard B-Tier chase behaviour
    Enemy::updateAI(deltaTime);
}

void Imp::performAttack() {
    if (lastAttackTime < attackCooldown)
        return;

    lastAttackTime = 0.0f;

    if (target && target->getIsAlive()) {
        int damage = attackDamage;

        // Imp doesn't hit hard, but fast
        target->takeDamage(damage);

        // Optional: mini knockback
        // target.applyKnockback(position, 12.0f);
    }
}

// Tier A Enemies

FallenShadowPaladin::FallenShadowPaladin(int playerLevel)
    : Enemy(EnemyType::FALLEN_SHADOW_PALADIN, 180 + playerLevel * 10, playerLevel,  // STRONGER
            "assets/sprite/fallen_shadow_paladin.png", "Fallen Shadow Paladin",
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

HarpyQueen::HarpyQueen(int playerLevel)
    : Enemy(EnemyType::HARPY_QUEEN,
            120 + playerLevel * 8,
            playerLevel,
            "assets/sprite/harpy.png",
            "Harpy Queen",
            120.0f,                  // extremely fast
            18 + playerLevel * 2,
            160.0f,
            55.0f),
      swoopCooldown(6.0f),
      lastSwoopTime(0.0f)
{
    tier = EnemyTier::A;
    attackCooldown = 2.5f;
    displayColor = Color{255, 200, 100, 255};
}

void HarpyQueen::updateAI(float deltaTime) {
    if (!target) return;

    lastSwoopTime += deltaTime;

    float distance = Vector2Distance(position, target->getPosition());

    // Special Aerial swoop attack
    if (lastSwoopTime >= swoopCooldown && distance < 200.0f) {
        Vector2 dir = Vector2Normalize({
            target->getPosition().x - position.x,
            target->getPosition().y - position.y
        });
        position.x += dir.x * 180.0f;
        position.y += dir.y * 180.0f;

        lastSwoopTime = 0;

        // Heavy dive attack
        target->takeDamage(attackDamage + 10);
        return;
    }

    Enemy::updateAI(deltaTime);
}

void HarpyQueen::performAttack() {
    if (lastAttackTime < attackCooldown) return;
    lastAttackTime = 0;

    if (target && target->getIsAlive()) {
        int damage = attackDamage;
        target->takeDamage(damage);
    }
}

Witch::Witch(int playerLevel)
    : Enemy(EnemyType::WITCH, 90 + playerLevel * 8, playerLevel,
            "assets/sprite/witch.png", "Witch",
            80.0f, 25 + playerLevel * 2, 150.0f, 120.0f) {
    tier = EnemyTier::A;
    attackCooldown = 2.2f;
}

void Witch::updateAI(float deltaTime) {
    Enemy::updateAI(deltaTime);
}

// Tier S

Necromancer::Necromancer(int playerLevel)
    : Enemy(EnemyType::NECROMANCER, 120 + playerLevel * 10, playerLevel,
            "assets/sprite/necromancer.png", "Necromancer",
            60.0f, 30 + playerLevel * 2, 160.0f, 140.0f) {
    tier = EnemyTier::S;
    attackCooldown = 3.0f;
}

void Necromancer::updateAI(float deltaTime) {
    // Could summon minions in future
    Enemy::updateAI(deltaTime);
}

void Necromancer::performAttack() {
    if (lastAttackTime < attackCooldown) return;
    lastAttackTime = 0;

    if (target && target->getIsAlive()) {
        target->takeDamage(attackDamage);
    }
}


std::unique_ptr<Enemy> Enemy::create(EnemyType type, int playerLevel) {
    switch (type) {

        // --- Tier D ---
        case EnemyType::GOBLIN: return std::make_unique<Goblin>(playerLevel);
        case EnemyType::SKELETON: return std::make_unique<Skeleton>(playerLevel);
        case EnemyType::SLIME: return std::make_unique<Slime>(playerLevel);
        case EnemyType::HOUND: return std::make_unique<FireHound>(playerLevel);
        case EnemyType::BAT: return std::make_unique<Bat>(playerLevel);
        case EnemyType::FIRE_SPIRIT: return std::make_unique<Fire_Spirit>(playerLevel);
        case EnemyType::DARK_SPIRIT: return std::make_unique<Dark_Spirit>(playerLevel);
        case EnemyType::LIGHT_SPIRIT: return std::make_unique<Light_Spirit>(playerLevel);

        // --- Tier C ---
        case EnemyType::CHIMERA_ANT: return std::make_unique<ChimeraAnt>(playerLevel);
        case EnemyType::WEREWOLF: return std::make_unique<Werewolf>(playerLevel);
        case EnemyType::CERBERUS: return std::make_unique<Cerberus>(playerLevel);
        case EnemyType::CYCLOPS: return std::make_unique<Cyclops>(playerLevel);
        case EnemyType::MINOTAUR: return std::make_unique<Minotaur>(playerLevel);
        case EnemyType::STONE_GOLEM: return std::make_unique<Stone_Golem>(playerLevel);
        case EnemyType::SALAMANDER_MAN: return std::make_unique<SalamanderMan>(playerLevel);
        case EnemyType::HONEY_BEE: return std::make_unique<HoneyBee>(playerLevel);
        case EnemyType::SKELETON_HOUND: return std::make_unique<SkeletonHound>(playerLevel);

        // --- Tier B ---
        case EnemyType::SKELETON_KNIGHT: return std::make_unique<Skeleton_Knight>(playerLevel);
        case EnemyType::ELF_GIRL: return std::make_unique<Elf_Girl>(playerLevel);
        case EnemyType::GOBLIN_GIANT: return std::make_unique<Goblin_Giant>(playerLevel);
        case EnemyType::MAGE: return std::make_unique<Mage>(playerLevel);
        case EnemyType::LAVA_GOLEM: return std::make_unique<Lava_Golem>(playerLevel);
        case EnemyType::IMP: return std::make_unique<Imp>(playerLevel);
        case EnemyType::ANCIENT_MUMMY: return std::make_unique<AncientMummy>(playerLevel);

        // --- Tier A ---
        case EnemyType::FALLEN_SHADOW_PALADIN: return std::make_unique<FallenShadowPaladin>(playerLevel);
        case EnemyType::HARPY_QUEEN: return std::make_unique<HarpyQueen>(playerLevel);
        case EnemyType::WITCH: return std::make_unique<Witch>(playerLevel);

        // --- Tier S ---
        case EnemyType::NECROMANCER: return std::make_unique<Necromancer>(playerLevel);

        default:
            return std::make_unique<Goblin>(playerLevel);
    }
}


