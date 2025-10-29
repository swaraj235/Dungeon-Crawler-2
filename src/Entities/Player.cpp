#include "Player.h"
#include "Config.h"
#include <cmath>
#include <iostream>
#include <algorithm>

Player::Player()
    : Character(Config::PLAYER_BASE_HEALTH, 1, "assets/sprites/player.png", "Hero"),
      speed(Config::PLAYER_BASE_SPEED), attackDamage(Config::PLAYER_BASE_DAMAGE),
      attackCooldown(Config::PLAYER_ATTACK_COOLDOWN), lastAttackTime(0),
      critChance(Config::PLAYER_CRIT_CHANCE), critMultiplier(Config::PLAYER_CRIT_MULTIPLIER),
      speedMultiplier(1.0f), maxInventorySize(24), speedBuffTime(0), stealthBuffTime(0),
      rageBuffTime(0), isStealthed(false) {

    position = {Config::SCREEN_WIDTH / 2.0f, Config::SCREEN_HEIGHT / 2.0f};
    updateAttackRange();

    // Initialize starting weapon
    currentWeapon = {WeaponType::WOODEN_SWORD, 10, 1.0f, 0.0f, "Wooden Sword"};

    std::cout << "Player initialized!" << std::endl;
}

Player::~Player() {}

void Player::update(float deltaTime) {
    if (!isAlive) return;

    lastAttackTime += deltaTime;

    // Update spell cooldowns
    for (auto& spell : spells) {
        spell.lastCastTime += deltaTime;
    }

    // Update buffs
    if (speedBuffTime > 0) speedBuffTime -= deltaTime;
    if (stealthBuffTime > 0) stealthBuffTime -= deltaTime;
    if (rageBuffTime > 0) rageBuffTime -= deltaTime;

    // Update stealth status
    isStealthed = (stealthBuffTime > 0);

    handleInput();
    updateAttackRange();
}

void Player::draw() {
    if (!isAlive) return;

    Color playerColor = WHITE;
    if (isStealthed) playerColor = Color{255, 255, 255, 100}; // Semi-transparent
    if (rageBuffTime > 0) playerColor = RED;

    if (sprite.id != 0) {
        DrawTexture(sprite, (int)position.x, (int)position.y, playerColor);
    } else {
        DrawRectangle((int)position.x, (int)position.y, 32, 32, BLUE);
    }

    // Draw health bar ABOVE player (new)
    float healthPercent = (float)health / maxHealth;
    Color healthColor = healthPercent > 0.5f ? LIME : (healthPercent > 0.25f ? ORANGE : RED);

    // Background
    DrawRectangle((int)position.x - 2, (int)position.y - 15, 36, 8, BLACK);
    // Health bar fill
    DrawRectangle((int)position.x, (int)position.y - 13, (int)(32 * healthPercent), 4, healthColor);
    // Border
    DrawRectangleLines((int)position.x - 2, (int)position.y - 15, 36, 8, WHITE);
}

void Player::handleInput() {
    Vector2 movement = {0, 0};

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) movement.y -= speed;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) movement.y += speed;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) movement.x -= speed;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) movement.x += speed;

    position.x += movement.x * GetFrameTime() * speedMultiplier;
    position.y += movement.y * GetFrameTime() * speedMultiplier;
}

void Player::attack() {
    if (lastAttackTime >= attackCooldown) {
        lastAttackTime = 0;
    }
}

int Player::computeAttackDamage() const {
    int baseDamage = attackDamage + currentWeapon.baseDamage + (level - 1) * Config::DAMAGE_PER_LEVEL;

    // Apply rage buff
    if (rageBuffTime > 0) {
        baseDamage = (int)(baseDamage * Config::RAGE_POTION_MULTIPLIER);
    }

    return baseDamage;
}

bool Player::canAttack() const {
    return lastAttackTime >= attackCooldown;
}

void Player::updateAttackRange() {
    float width = 84.0f;
    float height = 72.0f;
    Vector2 center = {position.x + 16.0f, position.y + 16.0f};

    attackRange = {
        center.x - width / 2.0f,
        center.y - height / 2.0f,
        width,
        height
    };
}

void Player::castSpell(SpellType type) {
    for (auto& spell : spells) {
        if (spell.type == type && spell.lastCastTime >= spell.cooldown) {
            spell.lastCastTime = 0;
            break;
        }
    }
}

bool Player::canCast(SpellType type) const {
    for (const auto& spell : spells) {
        if (spell.type == type) {
            return spell.lastCastTime >= spell.cooldown;
        }
    }
    return false;
}

void Player::gainExperience(int amount) {
    experience += amount;

    int expNeeded = (int)(Config::EXP_FOR_LEVEL_2 * pow(Config::EXP_SCALING, level - 1));

    if (experience >= expNeeded) {
        level++;
        experience = 0;

        maxHealth += Config::HEALTH_PER_LEVEL;
        health = maxHealth;
        attackDamage += Config::DAMAGE_PER_LEVEL;
        speed += Config::SPEED_PER_LEVEL;

        // Unlock items at certain levels
        if (level == 5) {
            spells.push_back({SpellType::FIREBALL, 2.0f, 2.0f, 0, "Firebolt"});
            std::cout << "Spell Unlocked: Firebolt!" << std::endl;
        }
        else if (level == 10) {
            spells.push_back({SpellType::CHAIN_LIGHTNING, 6.0f, 6.0f, 0, "Chain Lightning"});
            addItem("Scorching Gauntlet", 1);
            addItem("Seeds of Evolution", 5);
            std::cout << "Weapon Unlocked: Scorching Gauntlet!" << std::endl;
        }
        else if (level == 15) {
            spells.push_back({SpellType::FROST_NOVA, 8.0f, 8.0f, 0, "Frost Nova"});
        }
        else if (level == 20) {
            spells.push_back({SpellType::WHIRLWIND, 10.0f, 10.0f, 0, "Whirlwind"});
        }

        std::cout << "Level Up! Now level " << level << std::endl;
    }
}

void Player::addItem(const std::string& itemName, int quantity) {
    for (auto& item : inventory) {
        if (item.name == itemName) {
            item.quantity += quantity;
            return;
        }
    }

    if (inventory.size() < maxInventorySize) {
        inventory.push_back({itemName, quantity});
    }
}

bool Player::useItem(const std::string& itemName) {
    for (auto& item : inventory) {
        if (item.name == itemName && item.quantity > 0) {
            item.quantity--;

            if (itemName == "Health Potion") {
                heal(Config::HEALTH_POTION_HEAL);
            } else if (itemName == "Speed Potion") {
                applySpeedBuff(Config::SPEED_POTION_DURATION);
            } else if (itemName == "Stealth Potion") {
                applyStealthBuff(Config::STEALTH_POTION_DURATION);
            } else if (itemName == "Rage Potion") {
                applyRageBuff(Config::RAGE_POTION_DURATION);
            } else if (itemName == "Holy Water of Life") {
                heal(Config::HOLY_WATER_OF_LIFE_HEAL);
            }

            if (item.quantity <= 0) {
                inventory.erase(
                    std::remove_if(inventory.begin(), inventory.end(),
                        [&itemName](const InventoryItem& i) { return i.name == itemName && i.quantity <= 0; }),
                    inventory.end()
                );
            }
            return true;
        }
    }
    return false;
}

bool Player::hasItem(const std::string& itemName) const {
    for (const auto& item : inventory) {
        if (item.name == itemName && item.quantity > 0) {
            return true;
        }
    }
    return false;
}

void Player::applySpeedBuff(float duration) {
    speedBuffTime = std::max(speedBuffTime, duration);
}

void Player::applyStealthBuff(float duration) {
    stealthBuffTime = std::max(stealthBuffTime, duration);
}

void Player::applyRageBuff(float duration) {
    rageBuffTime = std::max(rageBuffTime, duration);
}

void Player::equipItem(ItemType item) {
    if (ItemSystem::isEquipment(item)) {
        equippedItems.push_back(item);
    }
}

void Player::unequipItem(ItemType item) {
    equippedItems.erase(
        std::remove(equippedItems.begin(), equippedItems.end(), item),
        equippedItems.end()
    );
}

bool Player::isItemEquipped(ItemType item) const {
    for (const auto& equipped : equippedItems) {
        if (equipped == item) return true;
    }
    return false;
}

void Player::activateShield() {
    if (isItemEquipped(ItemType::SHIELD_PENDANT)) {
        shieldActive = true;
        shieldDuration = 500.0f;
    }
}

void Player::drawShield() {
    if (shieldActive && shieldDuration > 0) {
        shieldDuration -= GetFrameTime();
        float alpha = (shieldDuration / 5.0f) * 255;
        DrawCircle((int)position.x + 16, (int)position.y + 16, 50,
                  Color{173, 216, 230, (unsigned char)alpha});
        DrawCircleLines((int)position.x + 16, (int)position.y + 16, 50,
                       Color{0, 191, 255, (unsigned char)alpha});
    }
}

