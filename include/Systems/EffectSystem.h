#pragma once
#include "raylib.h"
#include <vector>

enum class EffectType {
    // Spell effects (existing)
    FIREBALL,
    FROST_WAVE,
    CHAIN_LIGHTNING,
    WHIRLWIND_SPIN,
    SPELL_CAST_READY,
    SHIELD_ACTIVATE,
    ITEM_THROW,

    // NEW: Player attack animations
    ATTACK_SLASH,        // Melee sword arc sweep
    ATTACK_CRIT_SLASH,   // Crit version — bigger, gold

    // NEW: Spell projectile/travel effects
    FIREBALL_PROJECTILE, // Traveling fireball orb
    FROST_NOVA_BURST,    // Expanding icy ring burst
    LIGHTNING_BOLT,      // Crackling line between points
    WHIRLWIND_ACTIVE,    // Spinning blades around player

    // NEW: Enemy attack animations
    ENEMY_WINDUP,        // Pre-attack telegraph (pulsing red ring)
    ENEMY_HIT_IMPACT,    // Impact flash when enemy connects

    // NEW: Status / misc
    LEVEL_UP_BURST,      // Gold ring + sparkle on level up
    ENEMY_DEATH_BURST,   // Dramatic enemy death explosion

    // NEW: Per-class enemy attack animations
    ENEMY_ATK_SLASH,     // Melee arc (Goblin, Skeleton, Hound, Werewolf …)
    ENEMY_ATK_MAGIC,     // Expanding orb (Mage, Witch, Necromancer, Spirits)
    ENEMY_ATK_STOMP,     // Ground shockwave (Golem, Giant, Minotaur, Cyclops)
    ENEMY_ATK_BITE,      // Fang-snap V (Bat, Cerberus, ChimeraAnt)
    ENEMY_ATK_SHADOW,    // Dark implosion (Imp, Dark Spirit, Shadow Paladin)
    ENEMY_ATK_FIRE,      // Fire burst (Fire Spirit, Lava Golem, Fire Hound)

    // NEW: Player spells
    SHADOW_BURST_RING,   // Expanding dark ring from Shadow Burst
    BLINK_TRAIL,         // Ghost image left at blink origin
    BLINK_IMPACT,        // Explosive arrival slash at blink destination

    // NEW: Companion VFX
    COMPANION_SLASH,     // Purple slash arc when Paladin attacks
};

struct Effect {
    Vector2 position;
    Vector2 targetPosition;   // For projectile / directional effects
    EffectType type;
    float timeLeft;
    float duration;
    float size;
    float rotation;           // For slash arc, whirlwind
    Color tintColor;          // Per-effect color override

    Effect(Vector2 pos, EffectType t, float dur, float sz,
           Vector2 target = {0, 0}, Color tint = WHITE, float rot = 0.0f)
        : position(pos), targetPosition(target), type(t),
          timeLeft(dur), duration(dur), size(sz), rotation(rot), tintColor(tint) {}
};

class EffectSystem {
private:
    std::vector<Effect> effects;

public:
    // ── Original spell triggers ──────────────────────────────────────────
    void addFireball(Vector2 position);
    void addFrostWave(Vector2 position);
    void addChainLightning(Vector2 from, Vector2 to);
    void addWhirlwind(Vector2 position);
    void addSpellCastReady(Vector2 position);
    void addShieldActivate(Vector2 position);
    void addItemThrow(Vector2 from, Vector2 to);

    // ── NEW: Player attack animations ────────────────────────────────────
    // facingRight: true = arc sweeps right side, false = left side
    void addAttackSlash(Vector2 playerCenter, bool facingRight, bool isCrit = false);

    // ── NEW: Spell projectile / AOE animations ───────────────────────────
    // These replace the old static circle effects with travelling/dramatic versions
    void addFireballProjectile(Vector2 from, Vector2 to);
    void addFrostNovaBurst(Vector2 center);
    void addLightningBolt(Vector2 from, Vector2 to);
    void addWhirlwindActive(Vector2 center);

    // ── NEW: Enemy attack animations ─────────────────────────────────────
    void addEnemyWindup(Vector2 enemyCenter);          // Call ~0.35s before hit
    void addEnemyHitImpact(Vector2 hitPosition);       // Call on contact

    // ── NEW: Misc / status effects ───────────────────────────────────────
    void addLevelUpBurst(Vector2 playerCenter);
    void addEnemyDeathBurst(Vector2 position, Color enemyColor);

    // ── NEW: Per-class enemy attack animations ───────────────────────────
    // facingRight: used by slash / bite to pick swing side
    void addEnemyAttackSlash(Vector2 center, bool facingRight);
    void addEnemyAttackMagic(Vector2 center);
    void addEnemyAttackStomp(Vector2 center);
    void addEnemyAttackBite(Vector2 center, bool facingRight);
    void addEnemyAttackShadow(Vector2 center);
    void addEnemyAttackFire(Vector2 center);

    // ── NEW: Player Spell VFX ────────────────────────────────────────────────
    void addShadowBurstRing(Vector2 center);   // Shadow Burst expanding ring
    void addBlinkTrail(Vector2 origin);        // Ghost echo at departure point
    void addBlinkImpact(Vector2 dest);         // Explosive slash at arrival

    // ── NEW: Companion VFX ───────────────────────────────────────────────────
    void addCompanionSlash(Vector2 center, bool facingRight);    // Paladin purple attack arc

    // ── Core ─────────────────────────────────────────────────────────────
    void update(float deltaTime);
    void draw();
    void clear();
};
