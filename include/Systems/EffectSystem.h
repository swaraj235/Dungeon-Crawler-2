#pragma once
#include "raylib.h"
#include <vector>

enum class EffectType {
    FIREBALL,
    FROST_WAVE,
    CHAIN_LIGHTNING,
    WHIRLWIND_SPIN,
    SPELL_CAST_READY,  // ADD THESE
    SHIELD_ACTIVATE,
    ITEM_THROW
};
struct Effect {
    Vector2 position;
    EffectType type;
    float timeLeft;
    float duration;
    float size;

    Effect(Vector2 pos, EffectType t, float dur, float sz)
        : position(pos), type(t), timeLeft(dur), duration(dur), size(sz) {}
};

class EffectSystem {
private:
    std::vector<Effect> effects;

public:
    void addFireball(Vector2 position);
    void addFrostWave(Vector2 position);
    void addChainLightning(Vector2 from, Vector2 to);
    void addWhirlwind(Vector2 position);
    void addSpellCastReady(Vector2 position);
    void addShieldActivate(Vector2 position);
    void addItemThrow(Vector2 from, Vector2 to);

    void update(float deltaTime);
    void draw();
    void clear();
};
