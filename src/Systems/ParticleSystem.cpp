#include "ParticleSystem.h"
#include <random>
#include <algorithm>
#include <cmath>
#include "raymath.h"

void ParticleSystem::addExplosion(Vector2 position, Color color, int count) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0, 6.28f);
    std::uniform_real_distribution<float> speedDist(50, 150);
    std::uniform_real_distribution<float> lifeDist(0.5f, 1.5f);

    for (int i {0}; i < count; i++) {
        float angle = angleDist(gen);
        float speed = speedDist(gen);
        Vector2 velocity = {cos(angle) * speed, sin(angle) * speed};
        float life = lifeDist(gen);
        float size = 3.0f + (rand() % 3);
        particles.emplace_back(position, velocity, color, life, size);
    }
}

void ParticleSystem::addBlood(Vector2 position, int count) {
    for (int i = 0; i < count; i++) {
        float angle = (rand() % 360) * 3.14f / 180.0f;
        float speed = 30 + (rand() % 50);
        Vector2 velocity = {cos(angle) * speed, sin(angle) * speed};
        Color bloodColor = Color{150 + rand() % 50, 0, 0, 255};

        particles.emplace_back(position, velocity, bloodColor, 1.0f, 2.0f);
    }
}

void ParticleSystem::addMagic(Vector2 position, Color color, int count) {
    for (int i = 0; i < count; i++) {
        float angle = (rand() % 360) * 3.14f / 180.0f;
        float speed = 20 + (rand() % 40);
        Vector2 velocity = {cos(angle) * speed, sin(angle) * speed};

        particles.emplace_back(position, velocity, color, 2.0f, 1.5f);
    }
}

void ParticleSystem::addHeal(Vector2 position, int count) {
    for (int i = 0; i < count; i++) {
        float angle = (rand() % 360) * 3.14f / 180.0f;
        float speed = 10 + (rand() % 30);
        Vector2 velocity = {cos(angle) * speed, sin(angle) * speed};

        particles.emplace_back(position, velocity, GREEN, 1.5f, 1.0f);
    }
}

void ParticleSystem::update(float deltaTime) {
    for (auto it = particles.begin(); it != particles.end();) {
        it->life -= deltaTime;
        it->position.x += it->velocity.x * deltaTime;
        it->position.y += it->velocity.y * deltaTime;

        // Gravity
        it->velocity.y += 100 * deltaTime;

        // Friction
        it->velocity.x *= 0.98f;
        it->velocity.y *= 0.98f;

        // Fade out
        it->color.a = (unsigned char)(255 * (it->life / it->maxLife));

        if (it->life <= 0) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleSystem::draw() {
    for (const auto& particle : particles) {
        DrawCircleV(particle.position, particle.size, particle.color);
    }
}

void ParticleSystem::clear() {
    particles.clear();
}
