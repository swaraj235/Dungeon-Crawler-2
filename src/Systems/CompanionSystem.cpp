#include "CompanionSystem.h"
#include "Enemy.h"
#include <string>
#include <iostream>
#include <cmath>
#include <memory>

Companion::Companion(CompanionType t, int lvl)
    : type(t), health(150), maxHealth(150), level(lvl), position({0, 0}),
      isAlive(true), attackCooldown(2.0f), lastAttackTime(0) {

    if (type == CompanionType::FALLEN_SHADOW_PALADIN) {
        maxHealth = 150 + (lvl * 10);
        health = maxHealth;
        attackCooldown = 2.5f;
    }
}

void Companion::update(float deltaTime) {
    if (!isAlive) return;
    lastAttackTime += deltaTime;
}

void Companion::draw() {
    if (!isAlive) return;

    // Shadow Paladin has special shadow effect
    Color companionColor = Color{100, 255, 200, 255};

    if (type == CompanionType::FALLEN_SHADOW_PALADIN) {
        // Draw as shadow with transparency
        DrawRectangle((int)position.x, (int)position.y, 32, 32, Fade(companionColor, 0.7f));
        DrawRectangleLines((int)position.x, (int)position.y, 32, 32, Color{0, 255, 136, 255});

        // Draw "shadow" effect
        DrawCircleV({position.x + 16, position.y + 40}, 15, Fade(BLACK, 0.3f));
    } else {
        DrawRectangle((int)position.x, (int)position.y, 32, 32, companionColor);
        DrawRectangleLines((int)position.x, (int)position.y, 32, 32, Color{0, 255, 136, 255});

        // Health bar for other companions
        DrawRectangle((int)position.x - 2, (int)position.y - 15, 36, 8, BLACK);
        float healthPercent = (float)health / maxHealth;
        DrawRectangle((int)position.x, (int)position.y - 13, (int)(32 * healthPercent), 4, LIME);
        DrawRectangleLines((int)position.x - 2, (int)position.y - 15, 36, 8, WHITE);
    }
}

void Companion::attack(Enemy* enemy) {
    if (!enemy || !enemy->getIsAlive() || lastAttackTime < attackCooldown) return;

    lastAttackTime = 0;

    // Calculate damage based on companion type
    int baseDamage = 25;
    if (type == CompanionType::FALLEN_SHADOW_PALADIN) {
        baseDamage = 30 + (level * 2);
    }

    enemy->takeDamage(baseDamage);
}

void Companion::takeDamage(int damage) {
    // Shadow Paladin takes NO damage when tamed
    if (type == CompanionType::FALLEN_SHADOW_PALADIN) {
        return; // Invincible!
    }

    health = std::max(0, health - damage);
    if (health <= 0) {
        isAlive = false;
    }
}

void Companion::followPlayer(Vector2 playerPos) {
    if (!isAlive) return;

    // Keep companion near player (offset to the side)
    Vector2 targetPos = {playerPos.x - 50, playerPos.y};
    Vector2 direction = {targetPos.x - position.x, targetPos.y - position.y};
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance > 5.0f) {
        direction.x /= distance;
        direction.y /= distance;
        position.x += direction.x * 100.0f * (1.0f / 60.0f); // Smooth movement
        position.y += direction.y * 100.0f * (1.0f / 60.0f);
    }
}

std::string Companion::getName() const {
    switch (type) {
        case CompanionType::FALLEN_SHADOW_PALADIN: return "Shadow Paladin";
        default: return "Unknown";
    }
}

CompanionSystem::CompanionSystem() : hasCompanion(false) {}

void CompanionSystem::tameCompanion(CompanionType type, int playerLevel) {
    currentCompanion = std::make_unique<Companion>(type, playerLevel);
    hasCompanion = true;
    std::cout << "Tamed " << currentCompanion->getName() << "!" << std::endl;
}

void CompanionSystem::updateCompanion(float deltaTime) {
    if (hasCompanion && currentCompanion) {
        currentCompanion->update(deltaTime);
        if (!currentCompanion->getIsAlive()) {
            hasCompanion = false;
        }
    }
}

void CompanionSystem::drawCompanion() {
    if (hasCompanion && currentCompanion) {
        currentCompanion->draw();
    }
}

void CompanionSystem::releaseCompanion() {
    currentCompanion.reset();
    hasCompanion = false;
}
