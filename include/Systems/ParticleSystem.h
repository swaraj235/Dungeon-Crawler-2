#pragma once
#include "raylib.h"
#include <vector>

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float life;
    float maxLife;
    float size;

    Particle(Vector2 pos, Vector2 vel, Color col, float lifetime, float sz)
        : position(pos), velocity(vel), color(col), life(lifetime), maxLife(lifetime), size(sz) {}
};

class ParticleSystem {
private:
    std::vector<Particle> particles;

public:
    void addExplosion(Vector2 position, Color color, int count = 15);
    void addBlood(Vector2 position, int count = 8);
    void addMagic(Vector2 position, Color color, int count = 10);
    void addHeal(Vector2 position, int count = 5);

    void update(float deltaTime);
    void draw();
    void clear();

    int getParticleCount() const { return particles.size(); }
};
