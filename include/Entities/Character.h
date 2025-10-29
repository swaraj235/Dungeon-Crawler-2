#pragma once
#include "raylib.h"
#include <string>

class Character {
protected:
    int health;
    int maxHealth;
    int level;
    int experience;
    Vector2 position;
    std::string name;
    bool isAlive;
    Texture2D sprite;

public:
    Character(int hp, int lvl, const std::string& spritePath, const std::string& charName);
    virtual ~Character();

    // Pure virtual methods (must be implemented by derived classes)
    virtual void update(float deltaTime) = 0;
    virtual void draw() = 0;

    // Common functionality
    virtual void takeDamage(int damage);
    virtual void heal(int amount);
    bool checkCollision(const Character& other) const;
    virtual Rectangle getBounds() const;

    // Getters
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getLevel() const { return level; }
    int getExperience() const { return experience; }
    Vector2 getPosition() const { return position; }
    std::string getName() const { return name; }
    bool getIsAlive() const { return isAlive; }

    // Setters
    void setPosition(Vector2 pos) { position = pos; }
    void setHealth(int hp);
};
