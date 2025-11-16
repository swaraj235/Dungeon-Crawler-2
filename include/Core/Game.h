#pragma once
#include "raylib.h"
#include "Player.h"
#include "Enemy.h"
#include "MapGenerator.h"
#include "ParticleSystem.h"
#include "SoundManager.h"
#include "HUD.h"
#include "SaveSystem.h"
#include "MainMenu.h"
#include "EffectSystem.h"
#include "CompanionSystem.h"
#include <vector>
#include <memory>
#include <random>

struct DamageNumber {
    Vector2 position;
    int damage;
    float timeLeft;
    Color color;

    DamageNumber(Vector2 pos, int dmg, Color col)
        : position(pos), damage(dmg), timeLeft(1.0f), color(col) {}
};

class Game {
private:
    // Game state
    bool isRunning;
    bool isPaused;
    bool gameOver;
    float gameTime;
    int currentFloor;

    // Game objects
    std::unique_ptr<Player> player;
    std::unique_ptr<MapGenerator> gameMap;
    std::vector<std::unique_ptr<Enemy>> enemies;
    ParticleSystem particleSystem;
    EffectSystem effectSystem;
    SoundManager soundManager;
    CompanionSystem companionSystem;
    std::unique_ptr<HUD> hud;

    // Inventory
    bool inventoryOpen;

    // Camera
    Camera2D camera;
    float cameraShakeTime;
    float cameraShakeIntensity;

    // Game stats
    int score;
    int enemiesKilled;

    // Enemy spawning
    std::mt19937 rng;
    float enemySpawnTimer;
    int maxEnemies;

    // Visual effects
    std::vector<DamageNumber> damageNumbers;
    float attackFlashTimer;

    // Save system
    SaveData saveData;

public:
    Game();
    ~Game();

    void run();
    void update(float deltaTime);
    void draw();
    void cleanup();

private:
    void initialize();
    void handleInput();
    void handleSpells();

    void updatePlayer(float deltaTime);
    void updateEnemies(float deltaTime);
    void updateCamera();
    void updateDamageNumbers(float deltaTime);
    void updateParticles(float deltaTime);

    void checkPlayerAttack();
    void checkCollisions();
    void removeDeadEnemies();
    void spawnEnemies();
    void generateNewFloor();

    void castFireball();
    void castFrostWave();
    void castChainLightning();
    void castWhirlwind();

    void drawDamageNumbers();
    void generateItemDrops(Enemy* enemy);
    void drawCompanionInfo();
    void drawGameOver();
    void drawPauseMenu();

    void saveGame();
    void loadGame();

    // Utility
    EnemyType selectEnemyType(int playerLevel);
    int calculateMaxEnemies() const;
    bool shouldSpawnEnemy() const;

    //Main menu
    std::unique_ptr<MainMenu> mainMenu;
    MenuState gameMenuState;

public:
    // Getters for HUD
    int getScore() const { return score; }
    int getEnemiesKilled() const { return enemiesKilled; }
    int getCurrentFloor() const { return currentFloor; }
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const { return enemies; }
    MapGenerator* getMap() const { return gameMap.get(); }
    Player* getPlayer() const { return player.get(); }
    CompanionSystem& getCompanionSystem() { return companionSystem; }
    bool getInventoryOpen() const { return inventoryOpen; }
    void setInventoryOpen(bool open) { inventoryOpen = open; }
};
