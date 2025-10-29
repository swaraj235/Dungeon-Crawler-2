#include "CombatSystem.h"

void CombatSystem::playerAttack(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
                                ParticleSystem& particles, SoundManager& sounds) {
    if (!player.canAttack()) return;

    Rectangle attackRange = player.getAttackRange();
    bool hitAny = false;

    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive()) continue;

        if (CheckCollisionRecs(attackRange, enemy->getBounds())) {
            hitAny = true;

            // Damage calculation
            int baseDamage = player.computeAttackDamage();
            bool isCrit = (rand() % 100) < (player.getCritChance() * 100);
            int finalDamage = isCrit ? (int)(baseDamage * player.getCritMultiplier()) : baseDamage;

            enemy->takeDamage(finalDamage);
            enemy->flashHit();

            Vector2 center = {attackRange.x + attackRange.width / 2, attackRange.y + attackRange.height / 2};
            enemy->applyKnockback(center, 20.0f);

            // Effects
            particles.addBlood(enemy->getPosition(), 5);
            sounds.playSound(SoundType::ATTACK_SWORD);

            if (!enemy->getIsAlive()) {
                particles.addExplosion(enemy->getPosition(), ORANGE, 10);
                int expReward = enemy->getLevel() * 25;
                player.gainExperience(expReward);
            }
        }
    }

    if (hitAny) {
        player.attack();
    }
}

void CombatSystem::enemyAttack(Enemy& enemy, Player& player, ParticleSystem& particles, SoundManager& sounds) {
    if (!enemy.canAttack() || !player.getIsAlive()) return;

    Rectangle enemyBounds = enemy.getBounds();
    Rectangle playerBounds = player.getBounds();

    if (CheckCollisionRecs(enemyBounds, playerBounds)) {
        int damage = enemy.getAttackDamage() / 4;
        player.takeDamage(damage);

        particles.addBlood(player.getPosition(), 3);
        sounds.playSound(SoundType::PLAYER_HIT);
    }
}

void CombatSystem::addEntry(const std::string& text, Color color) {
    combatLog.push_back({text, 0, color});
}

const std::vector<CombatLogEntry>& CombatSystem::getLog() const {
    return combatLog;
}

void CombatSystem::clearLog() {
    combatLog.clear();
}
