#include "EffectSystem.h"
#include <cmath>

void EffectSystem::addSpellCastReady(Vector2 position) {
    effects.emplace_back(position, EffectType::SPELL_CAST_READY, 0.5f, 15.0f);
}

void EffectSystem::addShieldActivate(Vector2 position) {
    effects.emplace_back(position, EffectType::SHIELD_ACTIVATE, 1.0f, 40.0f);
}

void EffectSystem::addItemThrow(Vector2 from, Vector2 to) {
    effects.emplace_back(from, EffectType::ITEM_THROW, 0.3f, 5.0f);
}

void EffectSystem::addFireball(Vector2 position) {
    effects.emplace_back(position, EffectType::FIREBALL, 0.6f, 20.0f);
}

void EffectSystem::addFrostWave(Vector2 position) {
    effects.emplace_back(position, EffectType::FROST_WAVE, 0.8f, 30.0f);
}

void EffectSystem::addChainLightning(Vector2 from, Vector2 to) {
    effects.emplace_back(from, EffectType::CHAIN_LIGHTNING, 0.4f, 5.0f);
}

void EffectSystem::addWhirlwind(Vector2 position) {
    effects.emplace_back(position, EffectType::WHIRLWIND_SPIN, 1.0f, 40.0f);
}

void EffectSystem::update(float deltaTime) {
    for (auto it = effects.begin(); it != effects.end();) {
        it->timeLeft -= deltaTime;
        it->size += deltaTime * 50.0f;

        if (it->timeLeft <= 0) {
            it = effects.erase(it);
        } else {
            ++it;
        }
    }
}

void EffectSystem::draw() {
    for (const auto& effect : effects) {
        float progress = 1.0f - (effect.timeLeft / effect.duration);
        unsigned char alpha = (unsigned char)(255 * (1.0f - progress));

        switch (effect.type) {
            case EffectType::FIREBALL: {
                Color fireColor = Color{255, 165, 0, alpha};
                DrawCircleV(effect.position, effect.size * progress, fireColor);
                DrawCircleV(effect.position, effect.size * progress * 0.6f, Color{255, 255, 0, alpha});
                break;
            }

            case EffectType::CHAIN_LIGHTNING: {
                Color lightningColor = Color{200, 220, 255, alpha};
                DrawCircleV(effect.position, effect.size * progress, lightningColor);
                break;
            }

            case EffectType::FROST_WAVE: {
                Color frostColor = Color{0, 200, 255, alpha};
                DrawCircleLines((int)effect.position.x, (int)effect.position.y,
                               (int)(effect.size * progress), frostColor);
                DrawCircleLines((int)effect.position.x, (int)effect.position.y,
                               (int)(effect.size * progress * 0.6f), frostColor);
                break;
            }

            case EffectType::WHIRLWIND_SPIN: {
                Color windColor = Color{200, 100, 255, alpha};
                float rotation = progress * 360.0f * 4.0f;

                for (int i = 0; i < 8; i++) {
                    float angle = (i * 45.0f + rotation) * 3.14159f / 180.0f;
                    DrawLineEx(
                        effect.position,
                        {effect.position.x + cos(angle) * effect.size,
                         effect.position.y + sin(angle) * effect.size},
                        2.0f, windColor
                    );
                }
                break;
            }

            case EffectType::SPELL_CAST_READY: {
                Color readyColor = Color{0, 255, 136, alpha};
                DrawCircleLines((int)effect.position.x, (int)effect.position.y,
                               (int)(effect.size * progress), readyColor);
                DrawCircleLines((int)effect.position.x, (int)effect.position.y,
                               (int)(effect.size * progress * 0.6f), readyColor);
                break;
            }

            case EffectType::SHIELD_ACTIVATE: {
                Color shieldColor = Color{173, 216, 230, alpha};
                DrawCircle((int)effect.position.x, (int)effect.position.y,
                          (int)(effect.size * progress), Fade(shieldColor, 0.3f));
                DrawCircleLines((int)effect.position.x, (int)effect.position.y,
                               (int)(effect.size * progress), shieldColor);
                break;
            }

            case EffectType::ITEM_THROW: {
                Color itemColor = Color{255, 215, 0, alpha};
                DrawCircle((int)effect.position.x, (int)effect.position.y, 3, itemColor);
                break;
            }
        }
    }
}

void EffectSystem::clear() {
    effects.clear();
}

