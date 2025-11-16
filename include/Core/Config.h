#pragma once

// Game Configuration
namespace Config {
    // Window
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 800;
    constexpr int TARGET_FPS = 60;
    constexpr const char* GAME_TITLE = "Dungeon Crawler v2.0";

    // Player Stats
    constexpr int PLAYER_BASE_HEALTH = 150;
    constexpr int PLAYER_BASE_DAMAGE = 35;
    constexpr float PLAYER_BASE_SPEED = 200.0f;
    constexpr float PLAYER_ATTACK_COOLDOWN = 0.25f;
    constexpr float PLAYER_CRIT_CHANCE = 0.12f;
    constexpr float PLAYER_CRIT_MULTIPLIER = 1.8f;

    // Player Progression (per level)
    constexpr int HEALTH_PER_LEVEL = 35;
    constexpr int DAMAGE_PER_LEVEL = 6;
    constexpr float SPEED_PER_LEVEL = 3.0f;
    constexpr int EXP_FOR_LEVEL_2 = 100;
    constexpr float EXP_SCALING = 1.0f; // Each level needs 1.5x more exp

    // Combat
    constexpr float ATTACK_RANGE = 80.0f;
    constexpr float KNOCKBACK_FORCE = 20.0f;

    // Enemy Spawning
    constexpr float ENEMY_SPAWN_INTERVAL = 6.0f;
    constexpr int BASE_MAX_ENEMIES = 3;
    constexpr int MAX_ENEMY_CAP = 8;

    // Map
    constexpr int TILE_SIZE = 32;
    constexpr int MAP_WIDTH = 80;
    constexpr int MAP_HEIGHT = 50;
    constexpr int LEVELS_PER_FLOOR = 5; // New map every 5 levels

    // Potion Effects
    constexpr int HEALTH_POTION_HEAL = 75;
    constexpr int HOLY_WATER_OF_LIFE_HEAL = 500;
    constexpr float SPEED_POTION_DURATION = 8000.0f;
    constexpr float SPEED_POTION_MULTIPLIER = 1.5f;
    constexpr float STEALTH_POTION_DURATION = 6000.0f;
    constexpr float RAGE_POTION_DURATION = 1000.0f;
    constexpr float RAGE_POTION_MULTIPLIER = 1.3f;

    // Drop Rates (0.0 to 1.0)
    constexpr float HEALTH_POTION_DROP_RATE = 0.5f;
    constexpr float SPEED_POTION_DROP_RATE = 0.15f;
    constexpr float STEALTH_POTION_DROP_RATE = 0.10f;
    constexpr float RAGE_POTION_DROP_RATE = 0.10f;
    constexpr float WEAPON_DROP_RATE = 0.05f;
    constexpr float HOLY_WATER_OF_LIFE_DROP_RATE = 0.03f;

    // Save System
    constexpr const char* SAVE_FILE = "saves/savegame.json";
    constexpr const char* BACKUP_SAVE_FILE = "saves/savegame_backup.json";

    // Audio
    constexpr float MASTER_VOLUME = 0.7f;
    constexpr float MUSIC_VOLUME = 0.5f;
    constexpr float SFX_VOLUME = 0.8f;
}
