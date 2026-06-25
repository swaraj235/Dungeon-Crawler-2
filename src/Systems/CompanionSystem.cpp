#include "CompanionSystem.h"
#include "Enemy.h"
#include "MapGenerator.h"
#include "EffectSystem.h"
#include "ParticleSystem.h"
#include <string>
#include "Systems/EffectSystem.h"
#include <iostream>
#include <cmath>
#include <memory>

Companion::Companion(CompanionType t, int lvl)
    : type(t), health(150), maxHealth(150), level(lvl), position({0, 0}),
      isAlive(true), attackCooldown(2.0f), lastAttackTime(0), hasSprite(false), facingRight(true) {

    if (type == CompanionType::FALLEN_SHADOW_PALADIN) {
        maxHealth = 1500; // STATIC — does not scale with level
        health = maxHealth;
        attackCooldown = 2.0f;
        sprite = LoadTexture("assets/sprite/fallen_shadow_paladin.png");
        hasSprite = (sprite.id != 0);
    } else if (type == CompanionType::SEED_OF_EVOLUTION) {
        maxHealth = 80;
        health = maxHealth;
        attackCooldown = 0.4f; // Rapid attacks
        sprite = LoadTexture("assets/sprite/goblin.png");
        hasSprite = (sprite.id != 0);
    }
}

Companion::~Companion() {
    if (hasSprite) {
        UnloadTexture(sprite);
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
        if (hasSprite) {
            // Flip sprite based on facingRight
            Rectangle src = {
                facingRight ? 0.0f : (float)sprite.width,
                0.0f,
                facingRight ? (float)sprite.width : -(float)sprite.width,
                (float)sprite.height
            };
            DrawTextureRec(sprite, src, {position.x - sprite.width/2.0f, position.y - sprite.height/2.0f}, Fade(WHITE, 0.9f));
        } else {
            DrawRectangle((int)position.x, (int)position.y, 32, 32, Fade(companionColor, 0.7f));
            DrawRectangleLines((int)position.x, (int)position.y, 32, 32, Color{0, 255, 136, 255});
        }
        
        // Draw health bar for Paladin
        DrawRectangle((int)position.x - 10, (int)position.y - 25, 40, 6, BLACK);
        float healthPercent = (float)health / maxHealth;
        DrawRectangle((int)position.x - 8, (int)position.y - 24, (int)(36 * healthPercent), 4, LIME);

        // Draw "shadow" effect
        DrawCircleV({position.x, position.y + 20}, 15, Fade(BLACK, 0.3f));
    } else if (type == CompanionType::SEED_OF_EVOLUTION) {
        // Goblin Specimen — use goblin sprite or fallback
        if (hasSprite) {
            Rectangle src = {
                facingRight ? 0.0f : (float)sprite.width,
                0.0f,
                facingRight ? (float)sprite.width : -(float)sprite.width,
                (float)sprite.height
            };
            DrawTextureRec(sprite, src, {position.x - sprite.width/2.0f, position.y - sprite.height/2.0f}, WHITE);
        } else {
            DrawRectangle((int)position.x, (int)position.y, 20, 20, Color{100, 150, 50, 255});
            DrawRectangleLines((int)position.x, (int)position.y, 20, 20, GREEN);
        }

        // Health bar
        DrawRectangle((int)position.x - 2, (int)position.y - 10, 24, 6, BLACK);
        float healthPercent = (float)health / maxHealth;
        DrawRectangle((int)position.x, (int)position.y - 8, (int)(20 * healthPercent), 3, LIME);
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
        baseDamage = 90; // Static — not level-scaled
    } else if (type == CompanionType::SEED_OF_EVOLUTION) {
        baseDamage = 15;
    }

    enemy->takeDamage(baseDamage);
}

void Companion::takeDamage(int damage) {
    // Shadow Paladin takes nerfed damage (e.g. 50% damage reduction)
    if (type == CompanionType::FALLEN_SHADOW_PALADIN) {
        damage = damage / 2;
    }

    health = std::max(0, health - damage);
    if (health <= 0) {
        isAlive = false;
    }
}

void Companion::followPlayer(Vector2 playerPos, float deltaTime) {
    if (!isAlive) return;

    float dx = playerPos.x - position.x;
    float dy = playerPos.y - position.y;

    // Only move if outside the 100px square around the player
    if (std::abs(dx) > 100.0f || std::abs(dy) > 100.0f) {
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance < 0.1f) return;

        float speed = (type == CompanionType::SEED_OF_EVOLUTION) ? 220.0f : 200.0f;

        position.x += (dx / distance) * speed * deltaTime;
        position.y += (dy / distance) * speed * deltaTime;
    }
}

std::string Companion::getName() const {
    switch (type) {
        case CompanionType::FALLEN_SHADOW_PALADIN: return "Shadow Paladin";
        case CompanionType::SEED_OF_EVOLUTION: return "Goblin Specimen";
        default: return "Unknown";
    }
}

CompanionSystem::CompanionSystem() {}

void CompanionSystem::tameCompanion(CompanionType type, int playerLevel, Vector2 pos) {
    auto comp = std::make_unique<Companion>(type, playerLevel);
    // Directly set position to be beside the player
    if (pos.x != 0 || pos.y != 0) {
        comp->setPosition({pos.x + 60, pos.y});
    }
    std::cout << "Summoned " << comp->getName() << "!" << std::endl;
    companions.push_back(std::move(comp));
}

void CompanionSystem::updateCompanion(float deltaTime, Vector2 playerPos,
                                       std::vector<std::unique_ptr<Enemy>>& enemies,
                                       MapGenerator* map,
                                       EffectSystem* effectSystem) {
    for (auto it = companions.begin(); it != companions.end(); ) {
        if (!(*it)->getIsAlive()) {
            it = companions.erase(it);
        } else {
            (*it)->update(deltaTime);

            // Find nearest enemy within detection radius
            Enemy* nearestEnemy = nullptr;
            float minDist = 350.0f * 350.0f;

            for (auto& enemy : enemies) {
                if (!enemy->getIsAlive()) continue;
                Vector2 ePos = enemy->getPosition();
                Vector2 cPos = (*it)->getPosition();
                float dx = ePos.x - cPos.x;
                float dy = ePos.y - cPos.y;
                float distSq = dx*dx + dy*dy;
                if (distSq < minDist) {
                    minDist = distSq;
                    nearestEnemy = enemy.get();
                }
            }

            Vector2 currentPos = (*it)->getPosition();
            Vector2 newPos = currentPos;

            if (nearestEnemy) {
                Vector2 targetPos = nearestEnemy->getPosition();
                float dx = targetPos.x - currentPos.x;
                float dy = targetPos.y - currentPos.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                // Update facing direction
                (*it)->setFacingRight(dx >= 0);

                float attackRange = ((*it)->getType() == CompanionType::FALLEN_SHADOW_PALADIN) ? 70.0f : 35.0f;

                if (distance > attackRange) {
                    // Chase enemy
                    float speed = ((*it)->getType() == CompanionType::SEED_OF_EVOLUTION) ? 240.0f : 200.0f;
                    newPos = {
                        currentPos.x + (dx / distance) * speed * deltaTime,
                        currentPos.y + (dy / distance) * speed * deltaTime
                    };
                } else {
                    // In range — attack with VFX for Paladin
                    bool attacked = false;
                    // Check if attack cooldown is ready (lastAttackTime is updated in update())
                    // We call attack() which internally checks the cooldown
                    // To know if attack fired, check before and after lastAttackTime
                    float prevTime = (*it)->getLastAttackTime();
                    (*it)->attack(nearestEnemy);
                    if ((*it)->getLastAttackTime() == 0 && prevTime > 0) {
                        // Attack just fired
                        if ((*it)->getType() == CompanionType::FALLEN_SHADOW_PALADIN) {
                            // Trigger companion slash VFX and particles
                            if (effectSystem) {
                                effectSystem->addCompanionSlash((*it)->getPosition(), (*it)->getFacingRight());
                            }
                            nearestEnemy->flashHit(0.15f);
                        }
                    }
                }
            } else {
                // No enemies — follow player using bounding square
                float dx = playerPos.x - currentPos.x;
                float dy = playerPos.y - currentPos.y;
                (*it)->setFacingRight(dx >= 0);

                if (std::abs(dx) > 100.0f || std::abs(dy) > 100.0f) {
                    float distance = std::sqrt(dx * dx + dy * dy);
                    if (distance > 0.1f) {
                        float speed = ((*it)->getType() == CompanionType::SEED_OF_EVOLUTION) ? 240.0f : 200.0f;
                        newPos = {
                            currentPos.x + (dx / distance) * speed * deltaTime,
                            currentPos.y + (dy / distance) * speed * deltaTime
                        };
                    }
                }
            }

            // Apply wall collision if map is available
            if (map && (newPos.x != currentPos.x || newPos.y != currentPos.y)) {
                float compW = ((*it)->getType() == CompanionType::FALLEN_SHADOW_PALADIN) ? 32.0f : 20.0f;
                float compH = compW;
                Rectangle compBounds = {newPos.x - compW/2, newPos.y - compH/2, compW, compH};
                Vector2 delta = {newPos.x - currentPos.x, newPos.y - currentPos.y};
                Vector2 resolved = map->resolveCollision(compBounds, delta);
                newPos = {currentPos.x + resolved.x, currentPos.y + resolved.y};
            }

            (*it)->setPosition(newPos);
            ++it;
        }
    }
}

void CompanionSystem::drawCompanion() {
    for(auto& c : companions) {
        c->draw();
    }
}

void CompanionSystem::releaseCompanion() {
    // Release shadow paladin only if requested, or release all for now.
    for (auto it = companions.begin(); it != companions.end(); ) {
        if ((*it)->getType() == CompanionType::FALLEN_SHADOW_PALADIN) {
            it = companions.erase(it);
        } else {
            ++it;
        }
    }
}

void CompanionSystem::releaseAllCompanions() {
    companions.clear();
}
