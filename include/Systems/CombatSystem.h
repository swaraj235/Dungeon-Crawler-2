#pragma once
#include <vector>
#include "Player.h"
#include "Enemy.h"
#include "ParticleSystem.h"
#include "Audio/SoundManager.h"
#include <memory>

struct CombatLogEntry {
    std::string description;
    float timestamp;
    Color color;
};

class CombatSystem {
private:
    std::vector<CombatLogEntry> combatLog;
    float lastCombatLogTime;
public:
    // Combat resolution
    static void playerAttack(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies, ParticleSystem& particles, SoundManager& sounds);
    static void enemyAttack(Enemy& enemy, Player& player, ParticleSystem& particles, SoundManager& sounds);

    // Combat log
    void addEntry(const std::string& text, Color color = WHITE);
    const std::vector<CombatLogEntry>& getLog() const;
    void clearLog();
};
