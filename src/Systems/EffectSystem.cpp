#include "EffectSystem.h"
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  HELPERS
// ─────────────────────────────────────────────────────────────────────────────
static const float FX_PI = 3.14159265f;

// Ease-out quad: fast then slows down
static float easeOut(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }

// Ease-in quad: slow then speeds up
static float easeIn(float t) { return t * t; }

// Bounce between 0→1→0
static float pulse(float t) { return std::sin(t * FX_PI); }

// Lerp between two vectors
static Vector2 lerpV(Vector2 a, Vector2 b, float t) {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

// ─────────────────────────────────────────────────────────────────────────────
//  ORIGINAL EFFECT TRIGGERS (unchanged API, kept for compatibility)
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::addSpellCastReady(Vector2 position) {
    effects.emplace_back(position, EffectType::SPELL_CAST_READY, 0.5f, 15.0f);
}

void EffectSystem::addShieldActivate(Vector2 position) {
    effects.emplace_back(position, EffectType::SHIELD_ACTIVATE, 1.0f, 40.0f);
}

void EffectSystem::addItemThrow(Vector2 from, Vector2 to) {
    effects.emplace_back(from, EffectType::ITEM_THROW, 0.3f, 5.0f, to);
}

void EffectSystem::addFireball(Vector2 position) {
    // Replaced by addFireballProjectile; keep as static burst fallback
    effects.emplace_back(position, EffectType::FIREBALL, 0.5f, 25.0f);
}

void EffectSystem::addFrostWave(Vector2 position) {
    effects.emplace_back(position, EffectType::FROST_WAVE, 0.8f, 30.0f);
}

void EffectSystem::addChainLightning(Vector2 from, Vector2 to) {
    effects.emplace_back(from, EffectType::CHAIN_LIGHTNING, 0.35f, 5.0f, to);
}

void EffectSystem::addWhirlwind(Vector2 position) {
    effects.emplace_back(position, EffectType::WHIRLWIND_SPIN, 1.0f, 40.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  NEW PLAYER ATTACK ANIMATIONS
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::addAttackSlash(Vector2 playerCenter, bool facingRight, bool isCrit) {
    // Rotation stores: base angle of arc + direction flag in sign
    float baseAngle = facingRight ? 0.0f : 180.0f;
    float sz = isCrit ? 60.0f : 45.0f;
    Color tint = isCrit ? Color{255, 200, 50, 255} : Color{255, 255, 255, 255};
    EffectType t = isCrit ? EffectType::ATTACK_CRIT_SLASH : EffectType::ATTACK_SLASH;
    effects.emplace_back(playerCenter, t, 0.18f, sz, Vector2{0, 0}, tint, baseAngle);
}

// ─────────────────────────────────────────────────────────────────────────────
//  NEW SPELL PROJECTILE / AOE ANIMATIONS
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::addFireballProjectile(Vector2 from, Vector2 to) {
    effects.emplace_back(from, EffectType::FIREBALL_PROJECTILE, 0.45f, 10.0f, to,
                         Color{255, 120, 20, 255});
}

void EffectSystem::addFrostNovaBurst(Vector2 center) {
    effects.emplace_back(center, EffectType::FROST_NOVA_BURST, 0.7f, 8.0f,
                         Vector2{0, 0}, Color{80, 200, 255, 255});
}

void EffectSystem::addLightningBolt(Vector2 from, Vector2 to) {
    effects.emplace_back(from, EffectType::LIGHTNING_BOLT, 0.3f, 4.0f, to,
                         Color{200, 220, 255, 255});
}

void EffectSystem::addWhirlwindActive(Vector2 center) {
    effects.emplace_back(center, EffectType::WHIRLWIND_ACTIVE, 1.2f, 55.0f,
                         Vector2{0, 0}, Color{200, 100, 255, 255});
}

// ─────────────────────────────────────────────────────────────────────────────
//  NEW ENEMY ATTACK ANIMATIONS
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::addEnemyWindup(Vector2 enemyCenter) {
    effects.emplace_back(enemyCenter, EffectType::ENEMY_WINDUP, 0.4f, 20.0f,
                         Vector2{0, 0}, Color{255, 60, 60, 255});
}

void EffectSystem::addEnemyHitImpact(Vector2 hitPosition) {
    effects.emplace_back(hitPosition, EffectType::ENEMY_HIT_IMPACT, 0.15f, 18.0f,
                         Vector2{0, 0}, Color{255, 80, 80, 255});
}

// ─────────────────────────────────────────────────────────────────────────────
//  NEW MISC EFFECTS
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::addLevelUpBurst(Vector2 playerCenter) {
    effects.emplace_back(playerCenter, EffectType::LEVEL_UP_BURST, 1.0f, 50.0f,
                         Vector2{0, 0}, Color{255, 220, 50, 255});
}

void EffectSystem::addEnemyDeathBurst(Vector2 position, Color enemyColor) {
    effects.emplace_back(position, EffectType::ENEMY_DEATH_BURST, 0.5f, 30.0f,
                         Vector2{0, 0}, enemyColor);
}

// ─────────────────────────────────────────────────────────────────────────────
//  PER-CLASS ENEMY ATTACK TRIGGERS
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::addEnemyAttackSlash(Vector2 center, bool facingRight) {
    float base = facingRight ? 0.0f : 180.0f;
    effects.emplace_back(center, EffectType::ENEMY_ATK_SLASH, 0.22f, 38.0f,
                         Vector2{0,0}, Color{255, 80, 80, 255}, base);
}

void EffectSystem::addEnemyAttackMagic(Vector2 center) {
    effects.emplace_back(center, EffectType::ENEMY_ATK_MAGIC, 0.5f, 22.0f,
                         Vector2{0,0}, Color{160, 80, 255, 255});
}

void EffectSystem::addEnemyAttackStomp(Vector2 center) {
    effects.emplace_back(center, EffectType::ENEMY_ATK_STOMP, 0.55f, 28.0f,
                         Vector2{0,0}, Color{180, 120, 60, 255});
}

void EffectSystem::addEnemyAttackBite(Vector2 center, bool facingRight) {
    float base = facingRight ? 0.0f : 180.0f;
    effects.emplace_back(center, EffectType::ENEMY_ATK_BITE, 0.25f, 20.0f,
                         Vector2{0,0}, Color{200, 40, 40, 255}, base);
}

void EffectSystem::addEnemyAttackShadow(Vector2 center) {
    effects.emplace_back(center, EffectType::ENEMY_ATK_SHADOW, 0.45f, 30.0f,
                         Vector2{0,0}, Color{100, 0, 180, 255});
}

void EffectSystem::addEnemyAttackFire(Vector2 center) {
    effects.emplace_back(center, EffectType::ENEMY_ATK_FIRE, 0.4f, 25.0f,
                         Vector2{0,0}, Color{255, 100, 0, 255});
}

// ─────────────────────────────────────────────────────────────────────────────
//  NEW: PLAYER SPELL VFX
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::addShadowBurstRing(Vector2 center) {
    // Phase 1 — fast dark implosion gather (inward pulse)
    effects.emplace_back(center, EffectType::SHADOW_BURST_RING, 0.85f, 180.0f,
                         Vector2{0,0}, Color{110, 0, 200, 255});
}

void EffectSystem::addBlinkTrail(Vector2 origin) {
    // Ghost echo fading at origin
    effects.emplace_back(origin, EffectType::BLINK_TRAIL, 0.4f, 24.0f,
                         Vector2{0,0}, Color{120, 0, 180, 255});
}

void EffectSystem::addBlinkImpact(Vector2 dest) {
    // Explosive crit slash on arrival
    effects.emplace_back(dest, EffectType::BLINK_IMPACT, 0.35f, 55.0f,
                         Vector2{0,0}, Color{255, 255, 255, 255});
}

// ─────────────────────────────────────────────────────────────────────────────
//  NEW: COMPANION VFX
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::addCompanionSlash(Vector2 center, bool facingRight) {
    // Purple sweeping arc from the Paladin's attack
    float rotation = facingRight ? 0.0f : 180.0f;
    effects.emplace_back(center, EffectType::COMPANION_SLASH, 0.25f, 38.0f,
                         Vector2{0,0}, Color{160, 0, 255, 255}, rotation);
}



void EffectSystem::update(float deltaTime) {
    for (auto it = effects.begin(); it != effects.end();) {
        it->timeLeft -= deltaTime;

        // Projectile: advance position toward target
        if (it->type == EffectType::FIREBALL_PROJECTILE) {
            float t = 1.0f - (it->timeLeft / it->duration);
            it->position = lerpV(it->position, it->targetPosition, deltaTime * 6.0f);
            it->size += deltaTime * 8.0f; // grows as it travels
        }

        if (it->timeLeft <= 0) {
            it = effects.erase(it);
        } else {
            ++it;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DRAW
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::draw() {
    for (const auto& e : effects) {
        float progress  = 1.0f - (e.timeLeft / e.duration); // 0→1 over lifetime
        float remaining = e.timeLeft / e.duration;            // 1→0 over lifetime
        unsigned char alpha = (unsigned char)(255 * remaining);

        switch (e.type) {

        // ── OLD FIREBALL (static burst, kept as fallback) ─────────────────
        case EffectType::FIREBALL: {
            float r = easeOut(progress) * e.size;
            DrawCircleV(e.position, r, Color{255, 165, 0, alpha});
            DrawCircleV(e.position, r * 0.55f, Color{255, 255, 80, alpha});
            break;
        }

        // ── OLD CHAIN LIGHTNING (line arc) ────────────────────────────────
        case EffectType::CHAIN_LIGHTNING: {
            unsigned char a2 = alpha;
            DrawLineEx(e.position, e.targetPosition, 3.0f, Color{200, 220, 255, a2});
            DrawLineEx(e.position, e.targetPosition, 1.0f, Color{255, 255, 255, a2});
            // Crackle sparks
            for (int i = 0; i < 4; i++) {
                float frac = (i + 1) * 0.2f;
                Vector2 pt = lerpV(e.position, e.targetPosition, frac);
                float jitter = std::sin(progress * 80.0f + i) * 8.0f;
                DrawCircleV({pt.x + jitter, pt.y + jitter}, 3.0f,
                            Color{200, 220, 255, a2});
            }
            break;
        }

        // ── OLD FROST WAVE ────────────────────────────────────────────────
        case EffectType::FROST_WAVE: {
            float r1 = easeOut(progress) * e.size;
            float r2 = r1 * 0.6f;
            DrawCircleLines((int)e.position.x, (int)e.position.y, r1,
                            Color{0, 200, 255, alpha});
            DrawCircleLines((int)e.position.x, (int)e.position.y, r2,
                            Color{120, 240, 255, alpha});
            DrawCircleV(e.position, r2 * 0.3f, Color{200, 240, 255, (unsigned char)(alpha / 3)});
            break;
        }

        // ── OLD WHIRLWIND ─────────────────────────────────────────────────
        case EffectType::WHIRLWIND_SPIN: {
            float rot = progress * 360.0f * 4.0f;
            for (int i = 0; i < 8; i++) {
                float angle = (i * 45.0f + rot) * FX_PI / 180.0f;
                DrawLineEx(e.position,
                           {e.position.x + cosf(angle) * e.size,
                            e.position.y + sinf(angle) * e.size},
                           2.0f, Color{200, 100, 255, alpha});
            }
            break;
        }

        // ── OLD SPELL CAST READY ──────────────────────────────────────────
        case EffectType::SPELL_CAST_READY: {
            float r = pulse(progress) * e.size;
            DrawCircleLines((int)e.position.x, (int)e.position.y, r,
                            Color{0, 255, 136, alpha});
            break;
        }

        // ── OLD SHIELD ACTIVATE ───────────────────────────────────────────
        case EffectType::SHIELD_ACTIVATE: {
            float r = easeOut(progress) * e.size;
            DrawCircle((int)e.position.x, (int)e.position.y, r,
                       Fade(Color{173, 216, 230, 255}, 0.25f * remaining));
            DrawCircleLines((int)e.position.x, (int)e.position.y, r,
                            Color{173, 216, 230, alpha});
            break;
        }

        // ── OLD ITEM THROW ────────────────────────────────────────────────
        case EffectType::ITEM_THROW: {
            float t = progress;
            Vector2 pt = lerpV(e.position, e.targetPosition, t);
            DrawCircleV(pt, 4.0f, Color{255, 215, 0, alpha});
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ATTACK SLASH  (sweeping arc)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ATTACK_SLASH:
        case EffectType::ATTACK_CRIT_SLASH: {
            bool isCrit = (e.type == EffectType::ATTACK_CRIT_SLASH);

            // Arc sweeps 110° total.
            // e.rotation holds base angle (0° = right, 180° = left).
            float sweepTotal = 110.0f * FX_PI / 180.0f;
            float baseRad    = e.rotation * FX_PI / 180.0f;
            float half       = sweepTotal / 2.0f;

            // At progress 0 the arc is thin; grows to full by progress 0.5,
            // then fades out.
            float arcProgress  = easeOut(std::min(progress * 2.0f, 1.0f));
            float currentSweep = sweepTotal * arcProgress;
            float startAngle   = baseRad - half;
            float endAngle     = startAngle + currentSweep;

            float r1 = e.size;          // outer radius
            float r2 = e.size * 0.45f;  // inner radius (hollow arc)

            // Trails: draw multiple semi-transparent lines fanning the arc
            int segments = isCrit ? 18 : 12;
            for (int i = 0; i <= segments; i++) {
                float ang = startAngle + (endAngle - startAngle) * (i / (float)segments);
                float trailAlpha = (float)i / segments; // brighter at the leading edge

                Color c = e.tintColor;
                c.a = (unsigned char)(alpha * trailAlpha);

                Vector2 inner = {e.position.x + cosf(ang) * r2,
                                 e.position.y + sinf(ang) * r2};
                Vector2 outer = {e.position.x + cosf(ang) * r1,
                                 e.position.y + sinf(ang) * r1};
                DrawLineEx(inner, outer, isCrit ? 3.5f : 2.5f, c);
            }

            // Leading-edge glow dot
            Color glowColor = isCrit ? Color{255, 240, 100, alpha}
                                     : Color{255, 255, 255, alpha};
            DrawCircleV({e.position.x + cosf(endAngle) * r1,
                         e.position.y + sinf(endAngle) * r1},
                        isCrit ? 5.0f : 3.5f, glowColor);

            // Crit: extra inner flash ring
            if (isCrit && progress < 0.4f) {
                float flashAlpha = (unsigned char)(200 * (1.0f - progress / 0.4f));
                DrawCircleLines((int)e.position.x, (int)e.position.y,
                                r1 * 0.7f, Color{255, 220, 50, (unsigned char)flashAlpha});
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: FIREBALL PROJECTILE  (improved — flickering corona + trail)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::FIREBALL_PROJECTILE: {
            float flicker  = 0.85f + 0.15f * sinf(progress * 80.0f);
            float baseR    = e.size * flicker;

            // Outer heat corona — radius pulses with slight sine variation
            float coronaR = baseR * (1.8f + 0.3f * sinf(progress * 50.0f));
            DrawCircleV(e.position, coronaR, Color{255, 40, 0, (unsigned char)(alpha / 4)});

            // Mid flame band
            DrawCircleV(e.position, baseR * 1.1f, Color{255, 120, 10, (unsigned char)(alpha * 3 / 4)});
            // Inner orange
            DrawCircleV(e.position, baseR * 0.75f, Color{255, 180, 30, alpha});
            // Bright white-yellow core
            DrawCircleV(e.position, baseR * 0.35f, Color{255, 255, 180, alpha});

            // Travel direction trail  — 3 shrinking spheres behind current pos
            float ang = atan2f(e.targetPosition.y - e.position.y,
                               e.targetPosition.x - e.position.x);
            for (int i = 1; i <= 4; i++) {
                Vector2 behind = {e.position.x - cosf(ang) * (i * 7.0f),
                                  e.position.y - sinf(ang) * (i * 7.0f)};
                unsigned char ta = (unsigned char)(alpha * (1.0f - i * 0.22f));
                Color tc = i <= 2 ? Color{255, 140, 0, ta} : Color{200, 60, 0, ta};
                DrawCircleV(behind, baseR * (0.6f - i * 0.1f), tc);
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: FROST NOVA BURST  — actual snowflake symbol
        // ─────────────────────────────────────────────────────────────────
        case EffectType::FROST_NOVA_BURST: {
            // Snowflake grows outward then fades
            float armLen = easeOut(progress) * e.size * 20.0f;
            Color ic = e.tintColor;
            ic.a = alpha;

            // 6 main arms at 60° intervals
            for (int i = 0; i < 6; i++) {
                float armAng = (i * 60.0f) * FX_PI / 180.0f;
                Vector2 tip = {e.position.x + cosf(armAng) * armLen,
                               e.position.y + sinf(armAng) * armLen};
                DrawLineEx(e.position, tip, 2.5f, ic);

                // Two side branches at 60% of the arm length
                float branchLen = armLen * 0.38f;
                Vector2 branchBase = {e.position.x + cosf(armAng) * armLen * 0.60f,
                                      e.position.y + sinf(armAng) * armLen * 0.60f};
                for (int s = -1; s <= 1; s += 2) {
                    float bAng = armAng + s * 60.0f * FX_PI / 180.0f;
                    DrawLineEx(branchBase,
                               {branchBase.x + cosf(bAng) * branchLen,
                                branchBase.y + sinf(bAng) * branchLen},
                               2.0f, ic);
                }
                // Tiny tip diamond dot
                DrawCircleV(tip, 2.5f, Color{200, 240, 255, alpha});
            }

            // Hexagonal center ring (6 short lines connecting arm roots)
            float hexR = armLen * 0.18f;
            for (int i = 0; i < 6; i++) {
                float a1 = (i * 60.0f) * FX_PI / 180.0f;
                float a2 = ((i + 1) * 60.0f) * FX_PI / 180.0f;
                DrawLineEx({e.position.x + cosf(a1) * hexR, e.position.y + sinf(a1) * hexR},
                           {e.position.x + cosf(a2) * hexR, e.position.y + sinf(a2) * hexR},
                           1.5f, ic);
            }

            // Brief white flash at center on birth
            float cf = std::max(0.0f, 1.0f - progress * 3.5f);
            DrawCircleV(e.position, armLen * 0.15f * cf,
                        Color{200, 240, 255, (unsigned char)(200 * cf)});
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: LIGHTNING BOLT  (jagged line with glow)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::LIGHTNING_BOLT: {
            // Draw main bolt + several jittered copies for the jagged look
            Color boltColor = e.tintColor;
            boltColor.a = alpha;
            Color glowC = Color{255, 255, 255, (unsigned char)(alpha * 0.6f)};

            // Thick glow
            DrawLineEx(e.position, e.targetPosition, 5.0f, glowC);
            // Main bolt
            DrawLineEx(e.position, e.targetPosition, 2.5f, boltColor);

            // Jitter segments
            int segs = 6;
            Vector2 prev = e.position;
            for (int i = 1; i <= segs; i++) {
                float frac = (float)i / segs;
                Vector2 pt = lerpV(e.position, e.targetPosition, frac);
                float jitter = sinf(progress * 100.0f + i * 3.14f) * 10.0f;
                pt.x += jitter;
                pt.y += jitter * 0.5f;
                Color jc = boltColor;
                DrawLineEx(prev, pt, 1.5f, jc);
                prev = pt;
            }

            // Impact glow at target
            float impactR = pulse(progress) * 15.0f;
            DrawCircleV(e.targetPosition, impactR,
                        Color{200, 220, 255, (unsigned char)(alpha / 2)});
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: WHIRLWIND ACTIVE  (spinning blade vortex around player)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::WHIRLWIND_ACTIVE: {
            float rot = progress * 360.0f * 6.0f; // fast spin

            // Outer orbit blades
            int blades = 6;
            for (int i = 0; i < blades; i++) {
                float ang = (i * (360.0f / blades) + rot) * FX_PI / 180.0f;

                // Orbit position
                float orbitR = e.size * 0.8f;
                Vector2 bladePt = {e.position.x + cosf(ang) * orbitR,
                                   e.position.y + sinf(ang) * orbitR};

                // Draw each blade as a short thick line
                Vector2 bladeTip = {e.position.x + cosf(ang) * (orbitR + 15.0f),
                                    e.position.y + sinf(ang) * (orbitR + 15.0f)};
                Color bc = e.tintColor;
                bc.a = alpha;
                DrawLineEx(bladePt, bladeTip, 4.0f, bc);

                // Glow dot at tip
                DrawCircleV(bladeTip, 3.5f, Color{255, 200, 255, alpha});
            }

            // Inner swirl rings
            for (int ring = 0; ring < 3; ring++) {
                float r = e.size * (0.3f + ring * 0.2f);
                Color rc = e.tintColor;
                rc.a = (unsigned char)(alpha * (0.4f + ring * 0.2f));
                DrawCircleLines((int)e.position.x, (int)e.position.y, r, rc);
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ENEMY WINDUP  (pre-attack telegraph)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ENEMY_WINDUP: {
            // Pulsing red ring that gets brighter and larger as it reaches 0
            float urgency = 1.0f - remaining; // 0→1, ramps up

            // Ring grows as urgency increases
            float r = e.size * (0.5f + urgency * 0.7f);
            Color c = {255, (unsigned char)(60 - urgency * 60),
                       (unsigned char)(60 - urgency * 60), alpha};

            DrawCircleLines((int)e.position.x, (int)e.position.y, r, c);
            // Inner pulsing fill
            float fillAlpha = (unsigned char)(80 * pulse(urgency));
            DrawCircleV(e.position, r * 0.6f,
                        Color{255, 50, 50, (unsigned char)fillAlpha});

            // Exclamation: 3 ticks outward
            for (int i = 0; i < 3; i++) {
                float ang = (i * 120.0f + urgency * 90.0f) * FX_PI / 180.0f;
                Vector2 tick = {e.position.x + cosf(ang) * r,
                                e.position.y + sinf(ang) * r};
                DrawCircleV(tick, 3.0f, Color{255, 80, 80, alpha});
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ENEMY HIT IMPACT  (white burst on contact)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ENEMY_HIT_IMPACT: {
            // Fast expanding white ring + center flash
            float r = easeOut(progress) * e.size;

            DrawCircleV(e.position, r * 0.5f,
                        Color{255, 100, 100, (unsigned char)(alpha * 0.7f)});
            DrawCircleLines((int)e.position.x, (int)e.position.y, r,
                            Color{255, 200, 200, alpha});

            // 4 impact lines bursting outward
            for (int i = 0; i < 4; i++) {
                float ang = (i * 90.0f + 45.0f) * FX_PI / 180.0f;
                float len = easeOut(progress) * e.size * 1.2f;
                DrawLineEx(e.position,
                           {e.position.x + cosf(ang) * len,
                            e.position.y + sinf(ang) * len},
                           2.5f, Color{255, 160, 160, alpha});
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: LEVEL UP BURST  (gold expanding ring + sparkles)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::LEVEL_UP_BURST: {
            // Three expanding gold rings
            for (int i = 0; i < 3; i++) {
                float rp = std::max(0.0f, progress - i * 0.1f);
                float r  = easeOut(rp) * (e.size * (1.5f + i * 0.5f));
                unsigned char ra = (unsigned char)(alpha * (1.0f - i * 0.25f));
                DrawCircleLines((int)e.position.x, (int)e.position.y, r,
                                Color{255, 215, 0, ra});
            }

            // Radiating sparkle lines
            float sparkLen = easeOut(progress) * 35.0f;
            for (int i = 0; i < 12; i++) {
                float ang = (i * 30.0f + progress * 60.0f) * FX_PI / 180.0f;
                Vector2 start = {e.position.x + cosf(ang) * 10.0f,
                                 e.position.y + sinf(ang) * 10.0f};
                Vector2 end   = {e.position.x + cosf(ang) * (10.0f + sparkLen),
                                 e.position.y + sinf(ang) * (10.0f + sparkLen)};
                DrawLineEx(start, end, 2.0f, Color{255, 240, 100, alpha});
            }

            // Center flash (fades fast)
            float cf = std::max(0.0f, 1.0f - progress * 4.0f);
            DrawCircleV(e.position, 20.0f * cf, Color{255, 255, 200, (unsigned char)(200 * cf)});
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ENEMY DEATH BURST  (dramatic coloured explosion)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ENEMY_DEATH_BURST: {
            float r = easeOut(progress) * e.size;

            // Coloured shockwave ring
            Color c = e.tintColor;
            c.a = alpha;
            DrawCircleLines((int)e.position.x, (int)e.position.y, r, c);
            DrawCircleLines((int)e.position.x, (int)e.position.y, r * 0.65f, c);

            // Inner fill flash
            DrawCircleV(e.position, r * 0.4f,
                        Color{255, 255, 255, (unsigned char)(alpha * 0.4f)});

            // 6 fragment lines shooting outward
            for (int i = 0; i < 6; i++) {
                float ang = (i * 60.0f) * FX_PI / 180.0f;
                float fragLen = easeOut(progress) * e.size * 1.1f;
                Color fc = c;
                DrawLineEx(e.position,
                           {e.position.x + cosf(ang) * fragLen,
                            e.position.y + sinf(ang) * fragLen},
                           3.0f, fc);
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ENEMY ATTACK - SLASH (Melee arc)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ENEMY_ATK_SLASH: {
            float sweepTotal = 120.0f * FX_PI / 180.0f;
            float baseRad    = e.rotation * FX_PI / 180.0f;
            float half       = sweepTotal / 2.0f;
            float arcProgress  = easeOut(std::min(progress * 2.0f, 1.0f));
            float currentSweep = sweepTotal * arcProgress;
            float startAngle   = baseRad - half;
            float endAngle     = startAngle + currentSweep;

            float r = e.size;
            int segments = 12;
            for (int i = 0; i <= segments; i++) {
                float ang = startAngle + (endAngle - startAngle) * (i / (float)segments);
                float trailAlpha = (float)i / segments;
                Color c = e.tintColor;
                c.a = (unsigned char)(alpha * trailAlpha);
                Vector2 inner = {e.position.x + cosf(ang) * (r * 0.4f), e.position.y + sinf(ang) * (r * 0.4f)};
                Vector2 outer = {e.position.x + cosf(ang) * r, e.position.y + sinf(ang) * r};
                DrawLineEx(inner, outer, 2.5f, c);
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ENEMY ATTACK - MAGIC (Expanding orb)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ENEMY_ATK_MAGIC: {
            float r = easeOut(progress) * e.size;
            Color c = e.tintColor;
            c.a = alpha;
            DrawCircleLines((int)e.position.x, (int)e.position.y, r, c);
            DrawCircleV(e.position, r * 0.6f, Color{c.r, c.g, c.b, (unsigned char)(alpha * 0.5f)});
            
            // Magic sparks
            for(int i = 0; i < 5; i++) {
                float ang = (i * 72.0f + progress * 90.0f) * FX_PI / 180.0f;
                DrawCircleV({e.position.x + cosf(ang) * r * 1.2f, e.position.y + sinf(ang) * r * 1.2f}, 2.0f, c);
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ENEMY ATTACK - STOMP (Ground shockwave)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ENEMY_ATK_STOMP: {
            float r = easeOut(progress) * e.size;
            Color c = e.tintColor;
            c.a = alpha;
            // Draw an ellipse for a ground-pound feel
            DrawEllipseLines((int)e.position.x, (int)e.position.y + 10, r, r * 0.5f, c);
            DrawEllipseLines((int)e.position.x, (int)e.position.y + 10, r * 0.8f, r * 0.4f, c);
            
            // Dust kickup
            for(int i = 0; i < 6; i++) {
                float ang = (180.0f + i * 36.0f) * FX_PI / 180.0f; // upper half arc
                float d = r * 0.8f;
                DrawCircleV({e.position.x + cosf(ang) * d, e.position.y + 10 + sinf(ang) * (d * 0.5f)}, 3.0f, c);
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ENEMY ATTACK - BITE (Fang-snap V)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ENEMY_ATK_BITE: {
            float baseRad = e.rotation * FX_PI / 180.0f;
            // The jaws start open and snap shut at progress=0.5
            float snap = (progress < 0.5f) ? (1.0f - progress * 2.0f) : 0.0f; 
            float angleOpen = 45.0f * snap * FX_PI / 180.0f;
            
            Color c = e.tintColor;
            c.a = alpha;
            
            // Upper jaw
            Vector2 upBase = {e.position.x + cosf(baseRad - FX_PI/2) * 5, e.position.y + sinf(baseRad - FX_PI/2) * 5};
            Vector2 upTip = {e.position.x + cosf(baseRad - angleOpen) * e.size, e.position.y + sinf(baseRad - angleOpen) * e.size};
            DrawLineEx(upBase, upTip, 3.0f, c);
            
            // Lower jaw
            Vector2 lowBase = {e.position.x + cosf(baseRad + FX_PI/2) * 5, e.position.y + sinf(baseRad + FX_PI/2) * 5};
            Vector2 lowTip = {e.position.x + cosf(baseRad + angleOpen) * e.size, e.position.y + sinf(baseRad + angleOpen) * e.size};
            DrawLineEx(lowBase, lowTip, 3.0f, c);

            // Bite impact flash
            if (progress > 0.4f && progress < 0.6f) {
                DrawCircleV({e.position.x + cosf(baseRad) * (e.size * 0.8f), e.position.y + sinf(baseRad) * (e.size * 0.8f)}, 10.0f, Color{255, 255, 255, 180});
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ENEMY ATTACK - SHADOW (Dark implosion)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ENEMY_ATK_SHADOW: {
            // Shadow pulls IN instead of bursting OUT
            float r = easeIn(remaining) * e.size; // 1->0
            Color c = e.tintColor;
            c.a = alpha;
            
            DrawCircleV(e.position, r, c);
            DrawCircleV(e.position, r * 0.5f, Color{0, 0, 0, alpha});
            
            // Tendrils pulling in
            for(int i = 0; i < 8; i++) {
                float ang = (i * 45.0f - progress * 45.0f) * FX_PI / 180.0f;
                Vector2 start = {e.position.x + cosf(ang) * (r + 15.0f), e.position.y + sinf(ang) * (r + 15.0f)};
                Vector2 end = {e.position.x + cosf(ang) * r, e.position.y + sinf(ang) * r};
                DrawLineEx(start, end, 2.0f, c);
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  NEW: ENEMY ATTACK - FIRE (Fire burst)
        // ─────────────────────────────────────────────────────────────────
        case EffectType::ENEMY_ATK_FIRE: {
            float r = easeOut(progress) * e.size;
            Color c = e.tintColor;
            c.a = alpha;
            
            // Fiery explosion
            DrawCircleV(e.position, r, c);
            DrawCircleV(e.position, r * 0.7f, Color{255, 200, 0, alpha});
            
            // Random flame licks
            for(int i = 0; i < 6; i++) {
                float ang = (i * 60.0f + sinf(progress * 10.0f) * 20.0f) * FX_PI / 180.0f;
                Vector2 tip = {e.position.x + cosf(ang) * (r * 1.5f), e.position.y + sinf(ang) * (r * 1.5f)};
                DrawLineEx(e.position, tip, 3.0f, c);
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  SHADOW BURST RING — imploding gather then expanding dark ring
        // ─────────────────────────────────────────────────────────────────
        case EffectType::SHADOW_BURST_RING: {
            Color c = e.tintColor;
            if (progress < 0.25f) {
                // Implosion gather phase — dark ring pulls inward
                float t = progress / 0.25f;
                float r = e.size * (1.0f - t * 0.8f);
                c.a = (unsigned char)(180 * t);
                DrawCircleLines((int)e.position.x, (int)e.position.y, r, c);
                // Inner dark fill
                DrawCircleV(e.position, r * 0.4f, Color{0, 0, 0, (unsigned char)(120 * t)});
                // 8 tendrils rushing inward
                for (int i = 0; i < 8; i++) {
                    float ang = (i * 45.0f) * FX_PI / 180.0f;
                    float tendLen = r * 0.5f;
                    DrawLineEx(
                        {e.position.x + cosf(ang) * (r + tendLen), e.position.y + sinf(ang) * (r + tendLen)},
                        {e.position.x + cosf(ang) * r,             e.position.y + sinf(ang) * r},
                        2.5f, Color{c.r, c.g, c.b, (unsigned char)(150 * t)});
                }
            } else {
                // Burst expansion phase
                float t = (progress - 0.25f) / 0.75f;
                float outerR = easeOut(t) * e.size;
                float innerR = outerR * 0.7f;
                c.a = (unsigned char)(255 * (1.0f - t));
                // Two expanding rings
                DrawCircleLines((int)e.position.x, (int)e.position.y, outerR, c);
                DrawCircleLines((int)e.position.x, (int)e.position.y, innerR,
                                Color{c.r, c.g, c.b, (unsigned char)(c.a / 2)});
                // Dark centre void
                DrawCircleV(e.position, innerR * 0.3f, Color{0, 0, 0, (unsigned char)(180 * (1.0f - t))});
                // 12 radial spokes
                for (int i = 0; i < 12; i++) {
                    float ang = (i * 30.0f + t * 30.0f) * FX_PI / 180.0f;
                    DrawLineEx(
                        {e.position.x + cosf(ang) * innerR, e.position.y + sinf(ang) * innerR},
                        {e.position.x + cosf(ang) * outerR, e.position.y + sinf(ang) * outerR},
                        2.0f, Color{c.r, c.g, c.b, (unsigned char)(c.a * 0.8f)});
                }
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  BLINK TRAIL — ghost silhouette fading at departure point
        // ─────────────────────────────────────────────────────────────────
        case EffectType::BLINK_TRAIL: {
            Color c = e.tintColor;
            c.a = (unsigned char)(200 * remaining);
            // Silhouette box (approximate player size)
            DrawRectangle((int)e.position.x, (int)e.position.y, 32, 48,
                          Color{c.r, c.g, c.b, (unsigned char)(c.a / 2)});
            DrawRectangleLines((int)e.position.x, (int)e.position.y, 32, 48, c);
            // Scatter shadow shards upward
            for (int i = 0; i < 5; i++) {
                float ang = (90.0f + i * 18.0f - 36.0f) * FX_PI / 180.0f;
                float len = easeOut(progress) * (12.0f + i * 5.0f);
                DrawLineEx(
                    {e.position.x + 16, e.position.y + 24},
                    {e.position.x + 16 + cosf(ang) * len, e.position.y + 24 + sinf(ang) * len},
                    2.0f, Color{c.r, c.g, c.b, (unsigned char)(c.a * 0.7f)});
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  BLINK IMPACT — explosive crit slash on arrival
        // ─────────────────────────────────────────────────────────────────
        case EffectType::BLINK_IMPACT: {
            // White shockwave flash
            float r = easeOut(progress) * e.size;
            unsigned char wa = (unsigned char)(255 * remaining);
            DrawCircleV(e.position, r * 0.4f, Color{255, 255, 255, (unsigned char)(wa * 0.6f)});
            DrawCircleLines((int)e.position.x, (int)e.position.y, r, Color{255, 255, 255, wa});

            // 8 white impact lines
            for (int i = 0; i < 8; i++) {
                float ang = (i * 45.0f) * FX_PI / 180.0f;
                float len = easeOut(progress) * e.size * 1.1f;
                DrawLineEx(e.position,
                           {e.position.x + cosf(ang) * len, e.position.y + sinf(ang) * len},
                           3.5f, Color{255, 200, 255, wa});
            }

            // Purple crit arc sweeping 180°
            float sweepTotal = FX_PI;
            float arcP = easeOut(std::min(progress * 2.0f, 1.0f));
            float currentSweep = sweepTotal * arcP;
            int segs = 16;
            for (int i = 0; i <= segs; i++) {
                float ang = -FX_PI / 2.0f + currentSweep * (i / (float)segs);
                float trail = (float)i / segs;
                float ri = e.size * 0.4f, ro = e.size;
                DrawLineEx(
                    {e.position.x + cosf(ang) * ri, e.position.y + sinf(ang) * ri},
                    {e.position.x + cosf(ang) * ro, e.position.y + sinf(ang) * ro},
                    3.0f, Color{200, 80, 255, (unsigned char)(wa * trail)});
            }
            break;
        }

        // ─────────────────────────────────────────────────────────────────
        //  COMPANION SLASH — purple arc when Paladin attacks
        // ─────────────────────────────────────────────────────────────────
        case EffectType::COMPANION_SLASH: {
            float sweepTotal = 130.0f * FX_PI / 180.0f;
            float arcP = easeOut(std::min(progress * 2.0f, 1.0f));
            float startAngle = -sweepTotal / 2.0f;
            float endAngle   = startAngle + sweepTotal * arcP;
            float ri = e.size * 0.3f, ro = e.size;
            Color c = e.tintColor;
            int segs = 14;
            for (int i = 0; i <= segs; i++) {
                float ang = startAngle + (endAngle - startAngle) * (i / (float)segs);
                float trail = (float)i / segs;
                c.a = (unsigned char)(alpha * trail);
                DrawLineEx(
                    {e.position.x + cosf(ang) * ri, e.position.y + sinf(ang) * ri},
                    {e.position.x + cosf(ang) * ro, e.position.y + sinf(ang) * ro},
                    2.5f, c);
            }
            // Glow dot at arc tip
            DrawCircleV(
                {e.position.x + cosf(endAngle) * ro, e.position.y + sinf(endAngle) * ro},
                4.0f, Color{200, 100, 255, alpha});
            break;
        }

        } // end switch
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  CLEAR
// ─────────────────────────────────────────────────────────────────────────────

void EffectSystem::clear() {
    effects.clear();
}
