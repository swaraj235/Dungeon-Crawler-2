#include "Game.h"
#include "Config.h"
#include "WeaponSystem.h"
#include "PotionSystem.h"
#include "ItemSystem.h"
#include "MainMenu.h"
#include "raymath.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdio>

Game::Game() : isRunning(true), isPaused(false), gameOver(false), gameTime(0),
               currentFloor(1), score(0), enemiesKilled(0),
               rng(std::random_device{}()), enemySpawnTimer(0), maxEnemies(3),
               cameraShakeTime(0), cameraShakeIntensity(0), attackFlashTimer(0),
               inventoryOpen(false) {

    // ONLY initialize window, NOT the game!
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, Config::GAME_TITLE);
    SetTargetFPS(Config::TARGET_FPS);

    // Initialize menu FIRST
    mainMenu = std::make_unique<MainMenu>();
    gameMenuState = MenuState::MAIN_MENU;

    // DON'T call initialize() here - let menu handle it!
}

Game::~Game() {
    cleanup();
}

void Game::initialize() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, Config::GAME_TITLE);
    SetTargetFPS(Config::TARGET_FPS);

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Initialize systems
    player = std::make_unique<Player>();
    gameMap = std::make_unique<MapGenerator>(Config::MAP_WIDTH, Config::MAP_HEIGHT, Config::TILE_SIZE);
    hud = std::make_unique<HUD>(screenWidth, screenHeight);

    // Initialize audio and systems
    soundManager.initialize();
    WeaponSystem::initialize();
    PotionSystem::initialize();
    ItemSystem::initialize();

    // Generate first floor
    gameMap->generateFloor(currentFloor);

    // Set player starting position
    Vector2 startPos = gameMap->getRandomSpawnPosition();
    player->setPosition(startPos);

    // Initialize camera
    camera.target = player->getPosition();
    camera.offset = {screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Spawn initial enemies
    spawnEnemies();

    // Load save if exists
    if (SaveSystem::saveExists(Config::SAVE_FILE)) {
        loadGame();
    }

    std::cout << "Game initialized successfully!" << std::endl;
}

void Game::run() {
    while (!WindowShouldClose() && isRunning) {
        float deltaTime = GetFrameTime();

        if (gameMenuState == MenuState::MAIN_MENU ||
            gameMenuState == MenuState::NEW_GAME ||
            gameMenuState == MenuState::LOAD_GAME ||
            gameMenuState == MenuState::SETTINGS) {

            gameMenuState = mainMenu->update();

            if (gameMenuState == MenuState::PLAYING) {
                if (!player) {
                    // Check which option was selected
                    MenuState selectedMenu = mainMenu->getState();

                    if (selectedMenu == MenuState::NEW_GAME) {
                        // NEW GAME - Delete old save and start fresh
                        std::cout << "Starting NEW GAME..." << std::endl;

                        // DELETE the old save file
                        std::remove(Config::SAVE_FILE);

                        // Initialize fresh game
                        initialize();

                        // Set player name
                        player->playerName = mainMenu->getPlayerName();

                        // Reset to level 1
                        player->setExperience(0);  // Direct access or use existing method
                        currentFloor = 1;
                        player->setHealth(player->getMaxHealth());

                        std::cout << "New game created for: " << player->playerName << std::endl;
                    }
                    else if (selectedMenu == MenuState::LOAD_GAME) {
                        // RESUME GAME - Load existing save
                        std::cout << "Resuming saved game..." << std::endl;

                        initialize();

                        if (SaveSystem::saveExists(Config::SAVE_FILE)) {
                            loadGame();
                            std::cout << "Loaded game for: " << player->playerName << std::endl;
                        } else {
                            std::cout << "No save file found, starting fresh..." << std::endl;
                        }
                    }
                }
            }

            mainMenu->draw();
        } else {
            // Normal game loop
            handleInput();

            if (!isPaused && !gameOver) {
                update(deltaTime);
            }

            draw();
        }
    }
}

void Game::update(float deltaTime) {
    gameTime += deltaTime;
    enemySpawnTimer += deltaTime;

    // If inventory open, DON'T update game world BUT still allow input handling
    // This was wrong - we were returning completely!
    // So let me fix this properly:

    if (attackFlashTimer > 0) attackFlashTimer -= deltaTime;
    if (cameraShakeTime > 0) cameraShakeTime -= deltaTime;

    // ONLY skip player/enemy updates when inventory is open
    if (inventoryOpen) {
        return;  // This is correct - don't update gameplay
    }

    updatePlayer(deltaTime);
    updateEnemies(deltaTime);
    updateParticles(deltaTime);
    updateCamera();
    updateDamageNumbers(deltaTime);

    checkPlayerAttack();
    checkCollisions();
    removeDeadEnemies();

    if (player->getLevel() > currentFloor * Config::LEVELS_PER_FLOOR) {
        generateNewFloor();
    }

    if (shouldSpawnEnemy()) {
        spawnEnemies();
        enemySpawnTimer = 0;
    }

    maxEnemies = calculateMaxEnemies();

    if (!player->getIsAlive()) {
        gameOver = true;
    }
}

void Game::handleInput() {
    // Toggle inventory
    if (IsKeyPressed(KEY_I)) {
        inventoryOpen = !inventoryOpen;
    }

    if (inventoryOpen) {

        // Use selected item with ENTER
        if (IsKeyPressed(KEY_ENTER)) {
            const auto& inventory = player->getInventory();
            if (!inventory.empty() && hud->getSelectedInventoryItem() < (int)inventory.size()) {
                const auto& selectedItem = inventory[hud->getSelectedInventoryItem()];
                player->useItem(selectedItem.name);
                effectSystem.addSpellCastReady(player->getPosition());
            }
        }

        // Close inventory with ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            inventoryOpen = false;
        }

        return; // Don't process other controls while inventory is open
    }

    // Normal game controls (rest stays the same)
    if (IsKeyPressed(KEY_P)) {
        isPaused = !isPaused;
    }

    if (IsKeyPressed(KEY_Q)) {
        isRunning = false;
    }

    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }

    if (IsKeyPressed(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) {
        saveGame();
    }

    if (gameOver && IsKeyPressed(KEY_R)) {
        cleanup();
        initialize();
        gameOver = false;
    }

    if (!isPaused && !gameOver && player->getIsAlive()) {
        // Quick potion use
        if (IsKeyPressed(KEY_H)) player->useItem("Health Potion");
        if (IsKeyPressed(KEY_J)) player->useItem("Speed Potion");
        if (IsKeyPressed(KEY_K)) player->useItem("Stealth Potion");
        if (IsKeyPressed(KEY_L)) player->useItem("Rage Potion");

        handleSpells();
    }
}

void Game::handleSpells() {
    if (IsKeyPressed(KEY_ONE)) {
        castFireball();
    } else if (IsKeyPressed(KEY_TWO)) {
        castChainLightning();
    } else if (IsKeyPressed(KEY_THREE)) {
        castFrostWave();
    } else if (IsKeyPressed(KEY_FOUR)) {
        castWhirlwind();
    }
}

void Game::updatePlayer(float deltaTime) {
    if (!player->getIsAlive() || inventoryOpen) return;

    Vector2 oldPos = player->getPosition();
    player->update(deltaTime);
    Vector2 newPos = player->getPosition();

    // Collision with walls
    Vector2 movement = {newPos.x - oldPos.x, newPos.y - oldPos.y};
    Rectangle playerBounds = player->getBounds();

    Vector2 resolvedMovement = gameMap->resolveCollision(playerBounds, movement);
    player->setPosition({oldPos.x + resolvedMovement.x, oldPos.y + resolvedMovement.y});
}

void Game::updateEnemies(float deltaTime) {
    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive()) continue;

        enemy->setTarget(player.get());

        Vector2 oldPos = enemy->getPosition();
        enemy->update(deltaTime);
        Vector2 newPos = enemy->getPosition();

        // Collision with walls
        Vector2 movement = {newPos.x - oldPos.x, newPos.y - oldPos.y};
        Rectangle enemyBounds = enemy->getBounds();

        Vector2 resolvedMovement = gameMap->resolveCollision(enemyBounds, movement);
        enemy->setPosition({oldPos.x + resolvedMovement.x, oldPos.y + resolvedMovement.y});
    }
}

void Game::updateCamera() {
    Vector2 targetPos = player->getPosition();
    camera.target.x += (targetPos.x - camera.target.x) * 0.1f;
    camera.target.y += (targetPos.y - camera.target.y) * 0.1f;

    // Screen shake
    if (cameraShakeTime > 0) {
        float dx = ((float)(rand() % 200) - 100.0f) / 100.0f * cameraShakeIntensity;
        float dy = ((float)(rand() % 200) - 100.0f) / 100.0f * cameraShakeIntensity;
        camera.target.x += dx;
        camera.target.y += dy;
    }

    // Keep camera in bounds
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float mapWidth = gameMap->getMapWidth() * gameMap->getTileSize();
    float mapHeight = gameMap->getMapHeight() * gameMap->getTileSize();

    camera.target.x = std::clamp(camera.target.x, screenWidth / 2.0f, mapWidth - screenWidth / 2.0f);
    camera.target.y = std::clamp(camera.target.y, screenHeight / 2.0f, mapHeight - screenHeight / 2.0f);
}

void Game::updateDamageNumbers(float deltaTime) {
    for (auto it = damageNumbers.begin(); it != damageNumbers.end();) {
        it->timeLeft -= deltaTime;
        it->position.y -= 30.0f * deltaTime;
        it->color.a = (unsigned char)(255 * std::max(0.0f, it->timeLeft));

        if (it->timeLeft <= 0) {
            it = damageNumbers.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::updateParticles(float deltaTime) {
    particleSystem.update(deltaTime);
}

void Game::checkPlayerAttack() {
    if (!player->getIsAlive()) return;

    if (IsKeyPressed(KEY_SPACE) && player->canAttack()) {
        attackFlashTimer = 0.2f;

        Rectangle attackRange = player->getAttackRange();
        bool hitAny = false;

        std::uniform_real_distribution<float> roll(0.0f, 1.0f);
        int baseDamage = player->computeAttackDamage();
        bool crit = roll(rng) < 0.15f;
        int finalDamage = crit ? (int)(baseDamage * 1.8f) : baseDamage;

        for (auto& enemy : enemies) {
            if (!enemy->getIsAlive()) continue;

            if (CheckCollisionRecs(attackRange, enemy->getBounds())) {
                hitAny = true;
                enemy->takeDamage(finalDamage);

                if (enemy->getIsAlive()) {
                    enemy->flashHit();
                    Vector2 center = {attackRange.x + attackRange.width / 2, attackRange.y + attackRange.height / 2};
                    enemy->applyKnockback(center, 20.0f);
                }

                particleSystem.addBlood(enemy->getPosition(), 5);
                damageNumbers.emplace_back(Vector2{enemy->getPosition().x, enemy->getPosition().y - 10},
                                          finalDamage, crit ? ORANGE : RED);

                if (!enemy->getIsAlive()) {
                    particleSystem.addExplosion(enemy->getPosition(), ORANGE, 10);
                    int expReward = enemy->getLevel() * 25;
                    player->gainExperience(expReward);
                    score += enemy->getLevel() * 100;
                    enemiesKilled++;

                    damageNumbers.emplace_back(Vector2{enemy->getPosition().x + 15, enemy->getPosition().y - 15},
                                              expReward, YELLOW);
                    generateItemDrops(enemy.get());
                    // TAMING SYSTEM - Chance to tame Shadow Paladin at level 35+
                    if (enemy->getEnemyType() == EnemyType::FALLEN_SHADOW_PALADIN &&
                        player->getLevel() >= 35 && !companionSystem.hasActiveCompanion()) {

                        std::uniform_int_distribution<int> tamingChance(1, 100);
                        if (tamingChance(rng) <= 30) { // 30% tame chance
                            companionSystem.tameCompanion(CompanionType::FALLEN_SHADOW_PALADIN, player->getLevel());
                            particleSystem.addMagic(enemy->getPosition(), Color{100, 255, 200, 255}, 20);

                            // Show taming message
                            DrawText("TAMED! Shadow Paladin joins you!",
                                    GetScreenWidth() / 2 - 100, 100, 20, Color{0, 255, 136, 255});
                        }
                        }
                    // Item drops
                    std::uniform_int_distribution<int> dropChance(1, 100);
                    int chance = dropChance(rng);

                    if (chance <= 5) {
                        // 5% - Legendary item
                        std::vector<ItemType> legendaryItems = {
                            ItemType::CLOAK_OF_INVISIBILITY,
                            ItemType::MYSTICAL_RUNE,
                            ItemType::ANCIENT_KEY
                        };
                        std::uniform_int_distribution<int> legendaryDist(0, legendaryItems.size() - 1);
                        ItemType item = legendaryItems[legendaryDist(rng)];
                        player->addItem(ItemSystem::getItemName(item), 1);
                        particleSystem.addMagic(enemy->getPosition(), Color{255, 215, 0, 255}, 12);
                    }
                    else if (chance <= 15) {
                        // 10% - Epic magical item
                        std::vector<ItemType> epicItems = {
                            ItemType::RING_OF_FIRE,
                            ItemType::AMULET_OF_ICE,
                            ItemType::BOOTS_OF_SWIFTNESS,
                            ItemType::MAGIC_ORB,
                            ItemType::SHIELD_PENDANT
                        };
                        std::uniform_int_distribution<int> epicDist(0, epicItems.size() - 1);
                        ItemType item = epicItems[epicDist(rng)];
                        player->addItem(ItemSystem::getItemName(item), 1);
                        particleSystem.addMagic(enemy->getPosition(), Color{200, 0, 200, 255}, 10);
                    }
                    else if (chance <= 25) {
                        // 10% - Weapon drop
                        std::vector<ItemType> weapons = {
                            // ItemType::IRON_KATANA,
                            // ItemType::STEEL_DAGGER,
                            ItemType::THROWING_KNIFE,
                            ItemType::SHURIKEN
                        };
                        std::uniform_int_distribution<int> weaponDist(0, weapons.size() - 1);
                        ItemType weapon = weapons[weaponDist(rng)];
                        player->addItem(ItemSystem::getItemName(weapon), 1);
                        particleSystem.addMagic(enemy->getPosition(), Color{192, 192, 192, 255}, 8);
                    }
                    else if (chance <= 50) {
                        // 25% - Food items
                        std::vector<ItemType> foodItems = {
                            ItemType::MEAT,
                            ItemType::APPLE,
                            ItemType::BREAD,
                            ItemType::CHEESE
                        };
                        std::uniform_int_distribution<int> foodDist(0, foodItems.size() - 1);
                        ItemType food = foodItems[foodDist(rng)];
                        int quantity = foodDist(rng) % 3 + 1; // 1-3 quantity
                        player->addItem(ItemSystem::getItemName(food), quantity);
                        particleSystem.addHeal(enemy->getPosition(), 5);
                    }
                    else if (chance <= 70) {
                        // 20% - Potion drops
                        std::vector<ItemType> potions = {
                            ItemType::HEALTH_POTION,
                            ItemType::SPEED_POTION,
                            ItemType::STEALTH_POTION,
                            ItemType::RAGE_POTION,
                            ItemType::MANA_POTION
                        };
                        std::uniform_int_distribution<int> potionDist(0, potions.size() - 1);
                        ItemType potion = potions[potionDist(rng)];
                        player->addItem(ItemSystem::getItemName(potion), 1);
                    }
                }
            }
        }

        if (hitAny) {
            cameraShakeTime = 0.1f;
            cameraShakeIntensity = 5.0f;
            soundManager.playSound(SoundType::ATTACK_SWORD);
        }

        player->attack();
    }
}

void Game::checkCollisions() {
    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive() || !player->getIsAlive()) continue;

        if (player->checkCollision(*enemy)) {
            int contactDamage = std::max(1, enemy->getAttackDamage() / 50);
            player->takeDamage(contactDamage);
            soundManager.playSound(SoundType::PLAYER_HIT);
        }
    }
}

void Game::removeDeadEnemies() {
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
                      [](const std::unique_ptr<Enemy>& enemy) { return !enemy->getIsAlive(); }),
        enemies.end()
    );
}

void Game::spawnEnemies() {
    int currentCount = (int)enemies.size();
    int toSpawn = std::min(2, maxEnemies - currentCount);

    if (toSpawn <= 0) return;

    std::vector<Vector2> spawnPositions = gameMap->getSpawnPositions(toSpawn);

    for (int i = 0; i < toSpawn; i++) {
        EnemyType type = selectEnemyType(player->getLevel());
        auto enemy = Enemy::create(type, player->getLevel());

        if (enemy) {
            enemy->setPosition(spawnPositions[i]);
            enemies.push_back(std::move(enemy));
        }
    }
}

EnemyType Game::selectEnemyType(int playerLevel) {
    std::vector<EnemyType> availableTypes;

    // Tier D (Always available)
    availableTypes.insert(availableTypes.end(), {
        EnemyType::GOBLIN, EnemyType::SKELETON, EnemyType::SLIME,
        EnemyType::HOUND, EnemyType::BAT
    });

    // Tier C (Level 10+)
    if (playerLevel >= 10) {
        availableTypes.insert(availableTypes.end(), {
            EnemyType::CHIMERA_ANT, EnemyType::WEREWOLF, EnemyType::CERBERUS,
            EnemyType::GIANT_CENTIPEDE, EnemyType::GIANT_SNAKE, EnemyType::STONE_TROLL
        });
    }

    if (playerLevel >= 15) {
        std::uniform_int_distribution<int> bossChance(1, 100);
        if (bossChance(rng) <= 5) { // 5% chance for Shadow Paladin
            return EnemyType::FALLEN_SHADOW_PALADIN;
        }
    }

    std::uniform_int_distribution<size_t> dist(0, availableTypes.size() - 1);
    return availableTypes[dist(rng)];
}

int Game::calculateMaxEnemies() const {
    return std::min(8, 3 + player->getLevel() / 2);
}

bool Game::shouldSpawnEnemy() const {
    return enemySpawnTimer >= Config::ENEMY_SPAWN_INTERVAL && enemies.size() < maxEnemies;
}

void Game::castFireball() {
    if (!player->canCast(SpellType::FIREBALL)) return;

    Rectangle range = player->getAttackRange();
    int damage = player->computeAttackDamage() + 15;

    for (auto& enemy : enemies) {
        if (enemy->getIsAlive() && CheckCollisionRecs(range, enemy->getBounds())) {
            enemy->takeDamage(damage);
            enemy->flashHit(0.15f);
            particleSystem.addMagic(enemy->getPosition(), ORANGE, 10);
            damageNumbers.emplace_back(Vector2{enemy->getPosition().x, enemy->getPosition().y - 12},
                                      damage, ORANGE);
        }
    }

    player->castSpell(SpellType::FIREBALL);
    cameraShakeTime = 0.08f;
    cameraShakeIntensity = 4.0f;
    soundManager.playSound(SoundType::ATTACK_MAGIC);
}

void Game::castChainLightning() {
    if (!player->canCast(SpellType::CHAIN_LIGHTNING)) return;

    int maxTargets = 3;
    int damage = player->computeAttackDamage() + 12;
    Vector2 origin = {player->getPosition().x + 16, player->getPosition().y + 16};

    std::vector<Enemy*> availableTargets;
    for (auto& enemy : enemies) {
        if (enemy->getIsAlive()) availableTargets.push_back(enemy.get());
    }

    Vector2 currentPos = origin;
    for (int i = 0; i < maxTargets && !availableTargets.empty(); i++) {
        Enemy* nearest = nullptr;
        float minDist = 1e9f;

        for (Enemy* enemy : availableTargets) {
            Vector2 enemyPos = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
            float dx = enemyPos.x - currentPos.x;
            float dy = enemyPos.y - currentPos.y;
            float dist = dx * dx + dy * dy;

            if (dist < minDist) {
                minDist = dist;
                nearest = enemy;
            }
        }

        if (!nearest) break;

        nearest->takeDamage(damage);
        nearest->flashHit(0.1f);
        particleSystem.addMagic(nearest->getPosition(), YELLOW, 10);
        damageNumbers.emplace_back(Vector2{nearest->getPosition().x, nearest->getPosition().y - 12},
                                  damage, YELLOW);

        currentPos = {nearest->getPosition().x + 8, nearest->getPosition().y + 8};
        availableTargets.erase(std::remove(availableTargets.begin(), availableTargets.end(), nearest),
                              availableTargets.end());
    }

    player->castSpell(SpellType::CHAIN_LIGHTNING);
    cameraShakeTime = 0.1f;
    cameraShakeIntensity = 5.0f;
}

void Game::castFrostWave() {
    if (!player->canCast(SpellType::FROST_NOVA)) return;

    Vector2 playerPos = player->getPosition();
    float radius = 120.0f;
    int damage = player->computeAttackDamage() + 10;

    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive()) continue;

        Vector2 enemyCenter = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
        Vector2 playerCenter = {playerPos.x + 16, playerPos.y + 16};

        float dx = enemyCenter.x - playerCenter.x;
        float dy = enemyCenter.y - playerCenter.y;

        if (dx * dx + dy * dy <= radius * radius) {
            enemy->takeDamage(damage);
            enemy->flashHit(0.2f);
            enemy->applyKnockback(playerPos, 15.0f);
            particleSystem.addMagic(enemy->getPosition(), SKYBLUE, 8);
            damageNumbers.emplace_back(Vector2{enemy->getPosition().x, enemy->getPosition().y - 12},
                                      damage, SKYBLUE);
        }
    }

    player->castSpell(SpellType::FROST_NOVA);
    cameraShakeTime = 0.12f;
    cameraShakeIntensity = 6.0f;
    soundManager.playSound(SoundType::ATTACK_MAGIC);
}

void Game::castWhirlwind() {
    if (!player->canCast(SpellType::WHIRLWIND)) return;

    Vector2 playerPos = player->getPosition();
    float radius = 80.0f;
    int damage = player->computeAttackDamage() + 20;

    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive()) continue;

        Vector2 enemyCenter = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
        Vector2 playerCenter = {playerPos.x + 16, playerPos.y + 16};

        float dx = enemyCenter.x - playerCenter.x;
        float dy = enemyCenter.y - playerCenter.y;

        if (dx * dx + dy * dy <= radius * radius) {
            enemy->takeDamage(damage);
            enemy->flashHit(0.1f);
            enemy->applyKnockback(playerPos, 25.0f);
            particleSystem.addExplosion(enemy->getPosition(), RED, 8);
            damageNumbers.emplace_back(Vector2{enemy->getPosition().x, enemy->getPosition().y - 12},
                                      damage, RED);
        }
    }

    player->castSpell(SpellType::WHIRLWIND);
    cameraShakeTime = 0.15f;
    cameraShakeIntensity = 8.0f;
}

void Game::generateNewFloor() {
    currentFloor++;
    gameMap->generateFloor(currentFloor);
    enemies.clear();
    damageNumbers.clear();

    Vector2 newPos = gameMap->getRandomSpawnPosition();
    player->setPosition(newPos);

    spawnEnemies();

    std::cout << "Entered Floor " << currentFloor << std::endl;
}

void Game::generateItemDrops(Enemy* enemy) {
    std::uniform_int_distribution<int> dropChance(1, 100);
    int chance = dropChance(rng);

    // Essence Stones - Always drop
    int stoneCount = 1 + (rand() % 5);
    player->addItem("Essence Stone", stoneCount);

    // Special drops based on enemy type
    if (enemy->getEnemyType() == EnemyType::GIANT_SNAKE && enemiesKilled % 10 == 0) {
        player->addItem("Venom Sword", 1);
        particleSystem.addMagic(enemy->getPosition(), Color{0, 200, 0, 255}, 15);
    }

    if (enemy->getEnemyType() == EnemyType::FALLEN_SHADOW_PALADIN) {
        player->addItem("Demon King Long Sword", 1);
        player->addItem("Orb", 3);
        particleSystem.addMagic(enemy->getPosition(), Color{200, 0, 200, 255}, 20);
    }

    // Random loot
    if (chance <= 10) {
        // Rare items
        std::vector<ItemType> rareItems = {
            ItemType::HOLY_WATER_OF_LIFE,
            ItemType::STARDUST,
            ItemType::PALADIN_NECKLACE,
            ItemType::SEEDS_OF_EVOLUTION,
            ItemType::MYSTICAL_RUNE
        };
        std::uniform_int_distribution<int> rareDist(0, rareItems.size() - 1);
        ItemType item = rareItems[rareDist(rng)];
        player->addItem(ItemSystem::getItemName(item), 1);
        particleSystem.addMagic(enemy->getPosition(), Color{255, 215, 0, 255}, 12);
    }
    else if (chance <= 30) {
        // Orbs - Heavy currency
        int orbCount = 1 + (rand() % 3);
        player->addItem("Orb", orbCount);
        particleSystem.addMagic(enemy->getPosition(), Color{255, 200, 0, 255}, 8);
    }
    else if (chance <= 60) {
        // Potions
        std::vector<ItemType> potions = {
            ItemType::HEALTH_POTION,
            ItemType::SPEED_POTION,
            ItemType::STEALTH_POTION,
            ItemType::RAGE_POTION,
            ItemType::MANA_POTION
        };
        std::uniform_int_distribution<int> potionDist(0, potions.size() - 1);
        ItemType potion = potions[potionDist(rng)];
        player->addItem(ItemSystem::getItemName(potion), 1);
    }
    else if (chance <= 80) {
        // Food items
        std::vector<ItemType> foodItems = {
            ItemType::MEAT,
            ItemType::APPLE,
            ItemType::BREAD,
            ItemType::CHEESE
        };
        std::uniform_int_distribution<int> foodDist(0, foodItems.size() - 1);
        ItemType food = foodItems[foodDist(rng)];
        int quantity = 1 + (rand() % 3);
        player->addItem(ItemSystem::getItemName(food), quantity);
    }
}

void Game::draw() {
    BeginDrawing();
    ClearBackground(Color{20, 20, 30, 255});

    BeginMode2D(camera);

    // Draw map
    gameMap->draw();

    // Draw companion
    companionSystem.drawCompanion();

    // Draw player
    if (player->getIsAlive()) {
        player->draw();
    }

    // Draw attack flash
    if (attackFlashTimer > 0) {
        Rectangle attackRange = player->getAttackRange();
        Color flashColor = Color{255, 0, 0, (unsigned char)(100 * (attackFlashTimer / 0.2f))};
        DrawRectangleRec(attackRange, flashColor);
        DrawRectangleLinesEx(attackRange, 3, RED);
    }

    // Draw enemies
    for (const auto& enemy : enemies) {
        if (enemy->getIsAlive()) {
            enemy->draw();
        }
    }

    // Draw particles
    particleSystem.draw();

    // Draw damage numbers
    drawDamageNumbers();

    EndMode2D();

    // Draw HUD
    hud->draw(this, player.get());

    if (gameOver) {
        drawGameOver();
    } else if (isPaused) {
        drawPauseMenu();
    }

    EndDrawing();
}

void Game::drawCompanionInfo() {
    Companion* companion = companionSystem.getCompanion();
    if (!companion) return;

    int screenWidth = GetScreenWidth();
    Rectangle companionPanel = {(float)(screenWidth - 200), 270.0f, 180.0f, 100.0f};
    DrawRectangleRec(companionPanel, Fade(BLACK, 0.8f));
    DrawRectangleLinesEx(companionPanel, 2, Color{0, 255, 136, 255});

    int y = 280;
    DrawText("=== COMPANION ===", screenWidth - 195, y, 12, Color{0, 255, 136, 255});
    y += 18;

    std::string nameStr = companion->getName();
    DrawText(nameStr.c_str(), screenWidth - 195, y, 11, WHITE);
    y += 14;

    std::string levelStr = "Level: " + std::to_string(companion->getLevel());
    DrawText(levelStr.c_str(), screenWidth - 195, y, 10, YELLOW);
    y += 14;

    std::string healthStr = "HP: " + std::to_string(companion->getHealth()) + "/" + std::to_string(companion->getMaxHealth());
    DrawText(healthStr.c_str(), screenWidth - 195, y, 10, LIME);
}

void Game::drawDamageNumbers() {
    for (const auto& damage : damageNumbers) {
        std::string text = "-" + std::to_string(damage.damage);
        DrawText(text.c_str(), (int)damage.position.x, (int)damage.position.y, 16, damage.color);
    }
}

void Game::drawGameOver() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));

    std::string gameOverText = "GAME OVER";
    std::string finalScoreText = "Final Score: " + std::to_string(score);
    std::string killsText = "Enemies Killed: " + std::to_string(enemiesKilled);
    std::string levelText = "Floor Reached: " + std::to_string(currentFloor);
    std::string restartText = "Press R to Restart | Press Q to Quit";

    int y = screenHeight / 2 - 100;

    int gameOverWidth = MeasureText(gameOverText.c_str(), 48);
    DrawText(gameOverText.c_str(), (screenWidth - gameOverWidth) / 2, y, 48, RED);
    y += 60;

    int finalScoreWidth = MeasureText(finalScoreText.c_str(), 24);
    DrawText(finalScoreText.c_str(), (screenWidth - finalScoreWidth) / 2, y, 24, WHITE);
    y += 30;

    int killsWidth = MeasureText(killsText.c_str(), 24);
    DrawText(killsText.c_str(), (screenWidth - killsWidth) / 2, y, 24, WHITE);
    y += 30;

    int levelWidth = MeasureText(levelText.c_str(), 24);
    DrawText(levelText.c_str(), (screenWidth - levelWidth) / 2, y, 24, WHITE);
    y += 50;

    int restartWidth = MeasureText(restartText.c_str(), 18);
    DrawText(restartText.c_str(), (screenWidth - restartWidth) / 2, y, 18, YELLOW);
}

void Game::drawPauseMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));

    std::string pauseText = "PAUSED";
    std::string resumeText = "Press P to Resume | Ctrl+S to Save | Q to Quit";

    int pauseWidth = MeasureText(pauseText.c_str(), 36);
    DrawText(pauseText.c_str(), (screenWidth - pauseWidth) / 2, screenHeight / 2 - 20, 36, WHITE);

    int resumeWidth = MeasureText(resumeText.c_str(), 18);
    DrawText(resumeText.c_str(), (screenWidth - resumeWidth) / 2, screenHeight / 2 + 30, 18, YELLOW);
}

void Game::saveGame() {
    if (!player) return;

    saveData.playerLevel = player->getLevel();
    saveData.playerHealth = player->getHealth();
    saveData.playerMaxHealth = player->getMaxHealth();
    saveData.playerExperience = player->getExperience();
    saveData.score = score;
    saveData.enemiesKilled = enemiesKilled;
    saveData.currentFloor = currentFloor;
    saveData.playTime = gameTime;
    saveData.currentWeapon = player->getWeapon().name;
    saveData.highestFloor = std::max(saveData.highestFloor, currentFloor);
    saveData.lastSaveTime = SaveSystem::getCurrentTimestamp();

    // Save inventory
    for (const auto& item : player->getInventory()) {
        if (item.name == "Health Potion") saveData.potions.healthPotions = item.quantity;
        else if (item.name == "Speed Potion") saveData.potions.speedPotions = item.quantity;
        else if (item.name == "Stealth Potion") saveData.potions.stealthPotions = item.quantity;
        else if (item.name == "Rage Potion") saveData.potions.ragePotions = item.quantity;
    }

    SaveSystem::save(saveData, Config::SAVE_FILE);
    std::cout << "Game saved! Floor " << currentFloor << ", Level " << player->getLevel() << std::endl;
}

void Game::loadGame() {
    if (!SaveSystem::load(saveData, Config::SAVE_FILE)) {
        std::cout << "No save file found, starting new game" << std::endl;
        return;
    }

    if (!player) {
        std::cout << "Player not initialized, cannot load" << std::endl;
        return;
    }

    // Load player name
    player->playerName = saveData.playerName;  // ← ADD THIS

    player->setHealth(saveData.playerHealth);

    score = saveData.score;
    enemiesKilled = saveData.enemiesKilled;
    currentFloor = saveData.currentFloor;
    gameTime = saveData.playTime;

    std::cout << "Game loaded! Player: " << player->playerName << " | Floor " << currentFloor << ", Level " << player->getLevel() << std::endl;
}

void Game::cleanup() {
    enemies.clear();
    damageNumbers.clear();
    player.reset();
    gameMap.reset();
    hud.reset();

    CloseWindow();
    std::cout << "Game cleanup completed" << std::endl;
}
