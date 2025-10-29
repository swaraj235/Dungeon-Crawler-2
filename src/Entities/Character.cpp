#include "Character.h"
#include <iostream>

Character::Character(int hp, int lvl, const std::string& spritePath, const std::string& charName)
    : health(hp), maxHealth(hp), level(lvl), experience(0),
      position({0, 0}), name(charName), isAlive(true) {
    sprite = LoadTexture(spritePath.c_str());

    if (sprite.id == 0) {
        std::cout << "Warning: Could not load sprite: " << spritePath << std::endl;
    }
}

Character::~Character() {
    if (sprite.id != 0) {
        UnloadTexture(sprite);
    }
}

void Character::takeDamage(int damage) {
    health = std::max(0, health - damage);
    if (health <= 0) {
        isAlive = false;
    }
}

void Character::heal(int amount) {
    health = std::min(maxHealth, health + amount);
}

bool Character::checkCollision(const Character& other) const {
    return CheckCollisionRecs(getBounds(), other.getBounds());
}

Rectangle Character::getBounds() const {
    float width = (sprite.id != 0) ? (float)sprite.width : 32.0f;
    float height = (sprite.id != 0) ? (float)sprite.height : 32.0f;
    return Rectangle{position.x, position.y, width, height};
}

void Character::setHealth(int hp) {
    health = std::max(0, std::min(hp, maxHealth));
    isAlive = (health > 0);
}
