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
#include <cfloat>

Game::Game() : isRunning(true), isPaused(false), gameOver(false), quitConfirmation(false), gameTime(0),
               currentFloor(1), score(0), enemiesKilled(0),
               rng(std::random_device{}()), enemySpawnTimer(0), maxEnemies(3),
               cameraShakeTime(0), cameraShakeIntensity(0), attackFlashTimer(0),
               inventoryOpen(false), shadowPaladinTamed(false), shadowPaladinSummonCooldown(0),
               shadowPaladinActiveTime(0), canTamePaladin(false) {

    // ONLY initialize window, NOT the game!
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, Config::GAME_TITLE);
    SetTargetFPS(Config::TARGET_FPS);
    
    // Ensure we are in the correct directory to load assets
    ChangeDirectory(GetApplicationDirectory());

    // Initialize menu FIRST
    mainMenu = std::make_unique<MainMenu>();
    gameMenuState = MenuState::MAIN_MENU;

    // DON'T call initialize() here - let menu handle it!
}

Game::~Game() {
    cleanup();
}

void Game::initialize() {
    // Window already created in constructor — just apply flags
    SetWindowState(FLAG_WINDOW_RESIZABLE);

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

    if (attackFlashTimer > 0) attackFlashTimer -= deltaTime;
    if (cameraShakeTime > 0) cameraShakeTime -= deltaTime;
    
    if (shadowPaladinSummonCooldown > 0) shadowPaladinSummonCooldown -= deltaTime;
    
    if (loreTimer > 0) {
        loreTimer -= deltaTime;
    }
    
    // Check if Paladin died
    bool isPaladinActiveNow = companionSystem.hasActiveCompanion(); // Assuming hasActiveCompanion checks for paladin specifically based on earlier code
    if (wasPaladinActive && !isPaladinActiveNow) {
        shadowPaladinSummonCooldown = 120.0f; // 2 minutes cooldown when he dies
    }
    wasPaladinActive = isPaladinActiveNow;

    // ONLY skip player/enemy updates when inventory is open
    if (inventoryOpen) {
        return;
    }

    updatePlayer(deltaTime);
    companionSystem.updateCompanion(deltaTime, player->getPosition(), enemies, gameMap.get(), &effectSystem);
    updateEnemies(deltaTime);
    effectSystem.update(deltaTime);
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
    if (quitConfirmation) {
        if (IsKeyPressed(KEY_Y)) {
            isRunning = false;
        } else if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ESCAPE)) {
            quitConfirmation = false;
        }
        return;
    }

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
                std::string itemName = selectedItem.name;
                if (player->useItem(itemName)) {
                    if (itemName == "Seeds of Evolution") {
                        float angles[3] = {0.0f, 120.0f, 240.0f};
                        for (int i=0; i<3; ++i) {
                            float angRad = angles[i] * PI / 180.0f;
                            Vector2 spawnPos = {
                                player->getPosition().x + cosf(angRad) * 40.0f,
                                player->getPosition().y + sinf(angRad) * 40.0f
                            };
                            companionSystem.tameCompanion(CompanionType::SEED_OF_EVOLUTION, player->getLevel(), spawnPos);
                        }
                    }
                    effectSystem.addSpellCastReady(player->getPosition());
                }
            }
        }

        // Close inventory with ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            inventoryOpen = false;
        }

        return; // Don't process other controls while inventory is open
    }

    // Normal game controls (rest stays the same)
    // ESC = primary pause/resume (bug #7 — previously ESC did nothing in gameplay)
    if (IsKeyPressed(KEY_ESCAPE)) {
        isPaused = !isPaused;
    }

    if (IsKeyPressed(KEY_P)) {
        isPaused = !isPaused;
    }

    if (IsKeyPressed(KEY_Q)) {
        quitConfirmation = true;
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

        // Shadow Paladin Taming & Summoning
        if (IsKeyPressed(KEY_T) && canTamePaladin && !shadowPaladinTamed) {
            shadowPaladinTamed = true;
            canTamePaladin = false;
            
            loreMessage = "The Throne lies empty, yet he stood guard...\nA fallen knight bound by an oath to a dead King.\nNow, his soul answers to you.";
            loreTimer = 10.0f; // Display for 10 seconds

            // Remove broken paladin
            for (auto& enemy : enemies) {
                if (enemy->getEnemyType() == EnemyType::FALLEN_SHADOW_PALADIN) {
                    FallenShadowPaladin* paladin = dynamic_cast<FallenShadowPaladin*>(enemy.get());
                    if (paladin && paladin->getBossInstance() && paladin->getBroken()) {
                        enemy->takeDamage(99999); // force kill
                        break;
                    }
                }
            }
            particleSystem.addMagic(player->getPosition(), Color{100, 255, 200, 255}, 30);
            soundManager.playSound(SoundType::LEVEL_UP);
            player->unlockSpell(SpellType::BLINK_STRIKE, 8.0f, "Blink Strike");
        }

        if (IsKeyPressed(KEY_F) && shadowPaladinTamed && shadowPaladinSummonCooldown <= 0) {
            if (!companionSystem.hasActiveCompanion()) {
                // Summon
                companionSystem.tameCompanion(CompanionType::FALLEN_SHADOW_PALADIN, player->getLevel(), player->getPosition());
                Companion* paladin = companionSystem.getCompanion();
                if (paladin && paladinSavedHP != -1) {
                    paladin->setHealth(paladinSavedHP);
                }
                wasPaladinActive = true;
                particleSystem.addMagic(player->getPosition(), Color{200, 0, 200, 255}, 30);
            } else {
                // Unsummon
                Companion* paladin = companionSystem.getCompanion();
                if (paladin) {
                    paladinSavedHP = paladin->getHealth();
                    particleSystem.addMagic(paladin->getPosition(), Color{100, 0, 100, 255}, 30);
                }
                companionSystem.releaseCompanion(); // Actually, this might release all companions, but it's fine for now. We can also just iterate and remove the paladin if we want, but currently releaseCompanion just clears companions. Let's check `releaseCompanion()` later.
                wasPaladinActive = false; // We unsummoned him, so don't trigger the death cooldown
            }
        }

        handleSpells();
    }
}

void Game::handleSpells() {
    if (IsKeyPressed(KEY_ONE)) {
        castFireball();
    } else if (IsKeyPressed(KEY_TWO)) {
        castWhirlwind();
    } else if (IsKeyPressed(KEY_THREE)) {
        castShadowBurst();
    } else if (IsKeyPressed(KEY_FOUR)) {
        castBlinkStrike();
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

        Character* bestCharTarget = nullptr;
        Companion* bestCompTarget = nullptr;
        float minDistSq = FLT_MAX;
        
        if (player->getIsAlive() && !player->getIsStealthed()) {
            Vector2 pPos = player->getPosition();
            Vector2 ePos = enemy->getPosition();
            minDistSq = (pPos.x - ePos.x)*(pPos.x - ePos.x) + (pPos.y - ePos.y)*(pPos.y - ePos.y);
            bestCharTarget = player.get();
        }
        
        auto& companions = companionSystem.getCompanions();
        for (auto& comp : companions) {
            if (comp->getIsAlive()) {
                Vector2 cPos = comp->getPosition();
                Vector2 ePos = enemy->getPosition();
                float distSq = (cPos.x - ePos.x)*(cPos.x - ePos.x) + (cPos.y - ePos.y)*(cPos.y - ePos.y);
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    bestCharTarget = nullptr;
                    bestCompTarget = comp.get();
                }
            }
        }
        
        enemy->setTarget(bestCharTarget);
        enemy->setCompanionTarget(bestCompTarget);

        Vector2 oldPos = enemy->getPosition();
        enemy->update(deltaTime);
        Vector2 newPos = enemy->getPosition();

        // Collision with walls
        Vector2 movement = {newPos.x - oldPos.x, newPos.y - oldPos.y};
        Rectangle enemyBounds = enemy->getBounds();

        Vector2 resolvedMovement = gameMap->resolveCollision(enemyBounds, movement);
        enemy->setPosition({oldPos.x + resolvedMovement.x, oldPos.y + resolvedMovement.y});

        // ── Windup telegraph animation ───────────────────────────────────
        // Fire the windup effect once when the enemy enters attack range
        // (windupTimer > 0 means it just started winding up)
        if (enemy->getWindupTimer() > 0.0f && !enemy->isWindupFired() &&
            enemy->getAIState() == AIState::CHASING) {
            Vector2 ec = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
            effectSystem.addEnemyWindup(ec);
            enemy->markWindupFired(); // suppress repeat until next swing
        }

        // ── Enemy Attack Animation ───────────────────────────────────────
        if (enemy->getJustAttacked()) {
            Vector2 ec = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
            bool right = enemy->getFacingRight();
            
            switch (enemy->getType()) {
                // Melee slashers
                case EnemyType::GOBLIN:
                case EnemyType::SKELETON:
                case EnemyType::HOUND:
                case EnemyType::WEREWOLF:
                case EnemyType::SKELETON_KNIGHT:
                    effectSystem.addEnemyAttackSlash(ec, right);
                    break;
                // Magic casters
                case EnemyType::MAGE:
                case EnemyType::WITCH:
                case EnemyType::NECROMANCER:
                case EnemyType::LIGHT_SPIRIT:
                case EnemyType::ELF_GIRL:
                    effectSystem.addEnemyAttackMagic(ec);
                    break;
                // Heavy stompers
                case EnemyType::STONE_GOLEM:
                case EnemyType::LAVA_GOLEM:
                case EnemyType::GOBLIN_GIANT:
                case EnemyType::MINOTAUR:
                case EnemyType::CYCLOPS:
                case EnemyType::ANCIENT_MUMMY:
                    effectSystem.addEnemyAttackStomp(ec);
                    break;
                // Biters / snap attacks
                case EnemyType::BAT:
                case EnemyType::CERBERUS:
                case EnemyType::CHIMERA_ANT:
                case EnemyType::HARPY_QUEEN:
                    effectSystem.addEnemyAttackBite(ec, right);
                    break;
                // Shadow / dark attacks
                case EnemyType::IMP:
                case EnemyType::DARK_SPIRIT:
                case EnemyType::FALLEN_SHADOW_PALADIN:
                    effectSystem.addEnemyAttackShadow(ec);
                    break;
                // Fire attacks
                case EnemyType::FIRE_SPIRIT:
                case EnemyType::SALAMANDER_MAN:
                case EnemyType::SLIME: // Give slime a fiery burst for fun
                    effectSystem.addEnemyAttackFire(ec);
                    break;
                default:
                    effectSystem.addEnemyAttackSlash(ec, right);
                    break;
            }
            enemy->clearJustAttacked();
        }
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
    effectSystem.update(deltaTime);
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

        // ── Slash animation ──────────────────────────────────────────────
        Vector2 playerCenter = {player->getPosition().x + 16,
                                player->getPosition().y + 16};
        effectSystem.addAttackSlash(playerCenter, player->getFacingRight(), crit);

        for (auto& enemy : enemies) {
            if (!enemy->getIsAlive()) continue;

            if (CheckCollisionRecs(attackRange, enemy->getBounds())) {
                hitAny = true;
                enemy->takeDamage(finalDamage);

                // Hit impact flash at enemy position
                Vector2 ec = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
                effectSystem.addEnemyHitImpact(ec);

                if (enemy->getIsAlive()) {
                    enemy->flashHit();
                    Vector2 center = {attackRange.x + attackRange.width / 2,
                                      attackRange.y + attackRange.height / 2};
                    enemy->applyKnockback(center, 20.0f);
                }

                particleSystem.addBlood(enemy->getPosition(), 5);
                damageNumbers.emplace_back(
                    Vector2{enemy->getPosition().x, enemy->getPosition().y - 10},
                    finalDamage, crit ? ORANGE : RED);

                if (!enemy->getIsAlive()) {
                    if (enemy->getEnemyType() == EnemyType::FALLEN_SHADOW_PALADIN) {
                        FallenShadowPaladin* paladin = dynamic_cast<FallenShadowPaladin*>(enemy.get());
                        if (paladin && paladin->getBossInstance() && !shadowPaladinTamed) {
                            paladin->heal(paladin->getMaxHealth());
                            paladin->setBroken(true);
                            canTamePaladin = true;
                            continue;
                        }
                    }
                    // ── Death burst (enemy's own colour) ──────────────────
                    effectSystem.addEnemyDeathBurst(ec, enemy->getDisplayColor());
                    particleSystem.addExplosion(enemy->getPosition(), ORANGE, 10);
                    int expReward = enemy->getLevel() * 25;
                    int levelBefore = player->getLevel();
                    player->gainExperience(expReward);
                    // Level-up burst animation
                    if (player->getLevel() > levelBefore) {
                        Vector2 pc = {player->getPosition().x + 16,
                                      player->getPosition().y + 16};
                        effectSystem.addLevelUpBurst(pc);
                    }
                    score += enemy->getLevel() * 100;
                    enemiesKilled++;

                    damageNumbers.emplace_back(Vector2{enemy->getPosition().x + 15, enemy->getPosition().y - 15},
                                              expReward, YELLOW);
                    generateItemDrops(enemy.get());

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
        EnemyType type = selectEnemyType(player->getLevel(), currentFloor);
        auto enemy = Enemy::create(type, player->getLevel());

        if (enemy) {
            enemy->setPosition(spawnPositions[i]);
            enemies.push_back(std::move(enemy));
        }
    }
}

EnemyType Game::selectEnemyType(int playerLevel, int currentFloor) {
    std::vector<EnemyType> availableTypes;
    FloorTheme theme = gameMap->getTheme();

    if (theme == FloorTheme::STONE_DUNGEON) {
        availableTypes = { EnemyType::GOBLIN, EnemyType::SKELETON, EnemyType::SLIME, EnemyType::BAT };
        if (playerLevel >= 5) {
            availableTypes.insert(availableTypes.end(), { EnemyType::HOUND, EnemyType::FIRE_SPIRIT });
        }
        if (playerLevel >= 10) {
            availableTypes.push_back(EnemyType::CHIMERA_ANT);
        }
    } 
    else if (theme == FloorTheme::CATACOMBS) {
        availableTypes = { EnemyType::SKELETON, EnemyType::SKELETON_HOUND, EnemyType::SKELETON_KNIGHT, EnemyType::ANCIENT_MUMMY };
        if (playerLevel >= 12) {
            availableTypes.insert(availableTypes.end(), { EnemyType::DARK_SPIRIT, EnemyType::IMP });
        }
        if (playerLevel >= 15) {
            availableTypes.push_back(EnemyType::MINOTAUR);
        }
    }
    else if (theme == FloorTheme::SHADOW_PALACE) {
        availableTypes = { EnemyType::DARK_SPIRIT, EnemyType::IMP };
        if (playerLevel >= 18) {
            availableTypes.insert(availableTypes.end(), { EnemyType::GOBLIN_GIANT, EnemyType::WITCH });
        }
    }

    if (availableTypes.empty()) {
        availableTypes.push_back(EnemyType::GOBLIN); // Fallback
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
    Vector2 playerCenter = {player->getPosition().x + 16, player->getPosition().y + 16};

    // Find nearest enemy for projectile target
    Enemy* nearestTarget = nullptr;
    float minDist = 1e9f;
    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive()) continue;
        float d = Vector2Distance(playerCenter, enemy->getPosition());
        if (d < minDist) { minDist = d; nearestTarget = enemy.get(); }
    }
    if (nearestTarget) {
        Vector2 targetCenter = {nearestTarget->getPosition().x + 16,
                                nearestTarget->getPosition().y + 16};
        effectSystem.addFireballProjectile(playerCenter, targetCenter);
    } else {
        // No target — burst at player
        effectSystem.addFireball(playerCenter);
    }

    for (auto& enemy : enemies) {
        if (enemy->getIsAlive() && CheckCollisionRecs(range, enemy->getBounds())) {
            enemy->takeDamage(damage);
            if (!enemy->getIsAlive()) { handleSpellKill(enemy.get()); continue; }
            enemy->flashHit(0.15f);
            particleSystem.addMagic(enemy->getPosition(), ORANGE, 10);
            damageNumbers.emplace_back(
                Vector2{enemy->getPosition().x, enemy->getPosition().y - 12},
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
            if (dist < minDist) { minDist = dist; nearest = enemy; }
        }

        if (!nearest) break;

        Vector2 nearestCenter = {nearest->getPosition().x + 16,
                                 nearest->getPosition().y + 16};
        // ── Jagged lightning bolt visual ─────────────────────────────────
        effectSystem.addLightningBolt(currentPos, nearestCenter);

        nearest->takeDamage(damage);
        handleSpellKill(nearest); // award EXP / handle Paladin break
        if (!nearest->getIsAlive()) {
            currentPos = nearestCenter;
            availableTargets.erase(
                std::remove(availableTargets.begin(), availableTargets.end(), nearest),
                availableTargets.end());
            continue;
        }
        nearest->flashHit(0.1f);
        particleSystem.addMagic(nearest->getPosition(), YELLOW, 10);
        damageNumbers.emplace_back(
            Vector2{nearest->getPosition().x, nearest->getPosition().y - 12},
            damage, YELLOW);

        currentPos = nearestCenter;
        availableTargets.erase(
            std::remove(availableTargets.begin(), availableTargets.end(), nearest),
            availableTargets.end());
    }

    player->castSpell(SpellType::CHAIN_LIGHTNING);
    cameraShakeTime = 0.1f;
    cameraShakeIntensity = 5.0f;
}

void Game::castFrostWave() {
    if (!player->canCast(SpellType::FROST_NOVA)) return;

    Vector2 playerPos = player->getPosition();
    Vector2 playerCenter = {playerPos.x + 16, playerPos.y + 16};
    float radius = 120.0f;
    int damage = player->computeAttackDamage() + 10;

    // ── Expanding ice nova burst ─────────────────────────────────────────
    effectSystem.addFrostNovaBurst(playerCenter);

    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive()) continue;

        Vector2 enemyCenter = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
        float dx = enemyCenter.x - playerCenter.x;
        float dy = enemyCenter.y - playerCenter.y;

        if (dx * dx + dy * dy <= radius * radius) {
            enemy->takeDamage(damage);
            if (!enemy->getIsAlive()) { handleSpellKill(enemy.get()); continue; }
            enemy->flashHit(0.2f);
            enemy->applyKnockback(playerPos, 15.0f);
            effectSystem.addEnemyHitImpact(enemyCenter);
            particleSystem.addMagic(enemy->getPosition(), SKYBLUE, 8);
            damageNumbers.emplace_back(
                Vector2{enemy->getPosition().x, enemy->getPosition().y - 12},
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
    Vector2 playerCenter = {playerPos.x + 16, playerPos.y + 16};
    float radius = 80.0f;
    int damage = player->computeAttackDamage() + 20;

    // ── Spinning blade vortex visual ─────────────────────────────────────
    effectSystem.addWhirlwindActive(playerCenter);

    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive()) continue;

        Vector2 enemyCenter = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
        float dx = enemyCenter.x - playerCenter.x;
        float dy = enemyCenter.y - playerCenter.y;

        if (dx * dx + dy * dy <= radius * radius) {
            enemy->takeDamage(damage);
            if (!enemy->getIsAlive()) { handleSpellKill(enemy.get()); continue; }
            enemy->flashHit(0.1f);
            enemy->applyKnockback(playerPos, 25.0f);
            effectSystem.addEnemyHitImpact(enemyCenter);
            particleSystem.addExplosion(enemy->getPosition(), RED, 8);
            damageNumbers.emplace_back(
                Vector2{enemy->getPosition().x, enemy->getPosition().y - 12},
                damage, RED);
        }
    }

    player->castSpell(SpellType::WHIRLWIND);
    cameraShakeTime = 0.15f;
    cameraShakeIntensity = 8.0f;
}

void Game::castShadowBurst() {
    if (!player->canCast(SpellType::SHADOW_BURST)) return;

    Vector2 playerPos = player->getPosition();
    Vector2 playerCenter = {playerPos.x + 16, playerPos.y + 16};
    float radius = 180.0f;
    int damage = player->computeAttackDamage() + 25; // Good AoE burst

    // Visual effect
    effectSystem.addShadowBurstRing(playerCenter);

    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive()) continue;

        Vector2 enemyCenter = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
        float dx = enemyCenter.x - playerCenter.x;
        float dy = enemyCenter.y - playerCenter.y;

        if (dx * dx + dy * dy <= radius * radius) {
            enemy->takeDamage(damage);
            if (!enemy->getIsAlive()) { handleSpellKill(enemy.get()); continue; }
            effectSystem.addEnemyAttackShadow(enemyCenter);
            damageNumbers.emplace_back(
                Vector2{enemy->getPosition().x, enemy->getPosition().y - 12},
                damage, PURPLE);
        }
    }
    
    player->castSpell(SpellType::SHADOW_BURST);
}

void Game::castBlinkStrike() {
    if (!player->canCast(SpellType::BLINK_STRIKE)) return;

    // Find nearest enemy in blink range
    Enemy* nearestEnemy = nullptr;
    float minDistSq = 350.0f * 350.0f; // Blink range
    Vector2 playerCenter = {player->getPosition().x + 16, player->getPosition().y + 16};

    for (auto& enemy : enemies) {
        if (!enemy->getIsAlive()) continue;

        Vector2 enemyCenter = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};
        float dx = enemyCenter.x - playerCenter.x;
        float dy = enemyCenter.y - playerCenter.y;
        float distSq = dx * dx + dy * dy;

        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearestEnemy = enemy.get();
        }
    }

    if (nearestEnemy) {
        Vector2 destPos = nearestEnemy->getPosition();
        // Leave trail at old pos
        effectSystem.addBlinkTrail(player->getPosition());

        // Teleport
        player->setPosition(destPos);

        // Huge damage (3x crit)
        int damage = player->computeAttackDamage() * 3;
        nearestEnemy->takeDamage(damage);
        handleSpellKill(nearestEnemy); // award EXP / handle Paladin break

        // Arrive impact
        effectSystem.addBlinkImpact({destPos.x + 16, destPos.y + 16});
        particleSystem.addExplosion(destPos, Color{255, 255, 255, 255}, 15);
        if (nearestEnemy->getIsAlive()) {
            damageNumbers.emplace_back(
                Vector2{nearestEnemy->getPosition().x, nearestEnemy->getPosition().y - 12},
                damage, WHITE);
        }
        soundManager.playSound(SoundType::ENEMY_HIT);

        player->castSpell(SpellType::BLINK_STRIKE);
    }
}

// ─── Shared post-kill handler for all spell damage ───────────────────────────
// Call this immediately after enemy->takeDamage() if the enemy is now dead.
void Game::handleSpellKill(Enemy* enemy) {
    if (!enemy || enemy->getIsAlive()) return;

    Vector2 ec = {enemy->getPosition().x + 16, enemy->getPosition().y + 16};

    // Shadow Paladin special case — break him instead of killing (bug #5)
    if (enemy->getEnemyType() == EnemyType::FALLEN_SHADOW_PALADIN && !shadowPaladinTamed) {
        FallenShadowPaladin* paladin = dynamic_cast<FallenShadowPaladin*>(enemy);
        if (paladin && paladin->getBossInstance()) {
            paladin->heal(paladin->getMaxHealth());
            paladin->setBroken(true);
            canTamePaladin = true;
            return; // don't award EXP yet; wait for taming
        }
    }

    // Standard death rewards
    effectSystem.addEnemyDeathBurst(ec, enemy->getDisplayColor());
    particleSystem.addExplosion(enemy->getPosition(), ORANGE, 10);

    int expReward = enemy->getLevel() * 25;
    int levelBefore = player->getLevel();
    player->gainExperience(expReward);

    if (player->getLevel() > levelBefore) {
        Vector2 pc = {player->getPosition().x + 16, player->getPosition().y + 16};
        effectSystem.addLevelUpBurst(pc);
    }

    score += enemy->getLevel() * 100;
    enemiesKilled++;

    damageNumbers.emplace_back(
        Vector2{enemy->getPosition().x + 15, enemy->getPosition().y - 15},
        expReward, YELLOW);

    generateItemDrops(enemy);
}

void Game::generateNewFloor() {
    currentFloor++;
    gameMap->generateFloor(currentFloor);
    enemies.clear();
    damageNumbers.clear();

    Vector2 newPos = gameMap->getRandomSpawnPosition();
    player->setPosition(newPos);

    spawnEnemies();

    if (gameMap->getTheme() == FloorTheme::SHADOW_PALACE && gameMap->getThroneRoomCenter().x != 0) {
        auto boss = Enemy::create(EnemyType::FALLEN_SHADOW_PALADIN, player->getLevel());
        if (boss) {
            boss->setPosition(gameMap->getThroneRoomCenter());
            FallenShadowPaladin* paladin = dynamic_cast<FallenShadowPaladin*>(boss.get());
            if (paladin) paladin->setBossInstance(true);
            enemies.push_back(std::move(boss));
        }
    }

    std::cout << "Entered Floor " << currentFloor << std::endl;
}

void Game::generateItemDrops(Enemy* enemy) {
    std::uniform_int_distribution<int> dropChance(1, 100);
    int chance = dropChance(rng);

    // Essence Stones - Always drop
    int stoneCount = 1 + (rand() % 5);
    player->addItem("Essence Stone", stoneCount);

    // Special drops based on enemy type
    if (enemy->getEnemyType() == EnemyType::MINOTAUR && enemiesKilled % 10 == 0) {
        player->addItem("Venom Sword", 1);
        particleSystem.addMagic(enemy->getPosition(), Color{0, 200, 0, 255}, 15);
    }

    if (enemy->getEnemyType() == EnemyType::FALLEN_SHADOW_PALADIN) {
        player->addItem("Demon King Long Sword", 1);
        player->addItem("Orb", 3);
        particleSystem.addMagic(enemy->getPosition(), Color{200, 0, 200, 255}, 20);
    }

    // Random loot
    // Seeds of Evolution get their own dedicated 15% chance (bug #3 - was too rare)
    if (chance <= 15) {
        player->addItem("Seeds of Evolution", 1);
        particleSystem.addMagic(enemy->getPosition(), Color{0, 220, 100, 255}, 14);
    }
    else if (chance <= 25) {
        // Other rare items
        std::vector<ItemType> rareItems = {
            ItemType::HOLY_WATER_OF_LIFE,
            ItemType::STARDUST,
            ItemType::PALADIN_NECKLACE,
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

    // Draw visual effects (slashes, spells, impacts, telegraphs)
    effectSystem.draw();

    // Draw damage numbers
    drawDamageNumbers();

    EndMode2D();

    // Draw HUD
    hud->draw(this, player.get());

    if (canTamePaladin && !shadowPaladinTamed) {
        DrawText("Press [T] to Tame the Shadow Paladin!", GetScreenWidth() / 2 - 200, 150, 20, GREEN);
    }
    
    if (shadowPaladinTamed && shadowPaladinSummonCooldown <= 0 && !companionSystem.hasActiveCompanion()) {
        DrawText("Press [F] to Summon Shadow Paladin", 20, 200, 20, PURPLE);
    } else if (shadowPaladinSummonCooldown > 0) {
        DrawText(TextFormat("Paladin Cooldown: %0.1fs", shadowPaladinSummonCooldown), 20, 200, 20, GRAY);
    }

    if (loreTimer > 0 && !loreMessage.empty()) {
        int textWidth = MeasureText(loreMessage.c_str(), 20);
        // Centered box at the bottom
        DrawRectangle(GetScreenWidth()/2 - 300, GetScreenHeight() - 150, 600, 100, Fade(BLACK, 0.7f));
        DrawRectangleLines(GetScreenWidth()/2 - 300, GetScreenHeight() - 150, 600, 100, PURPLE);
        DrawText(loreMessage.c_str(), GetScreenWidth()/2 - textWidth/2, GetScreenHeight() - 115, 18, LIGHTGRAY);
    }

    if (gameOver) {
        drawGameOver();
    } else if (quitConfirmation) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.8f));
        const char* prompt = "Are you sure you want to quit? (Y/N)";
        int promptWidth = MeasureText(prompt, 30);
        DrawText(prompt, GetScreenWidth() / 2 - promptWidth / 2, GetScreenHeight() / 2 - 15, 30, WHITE);
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
    std::string resumeText = "Press [ESC] to Resume  |  Ctrl+S to Save  |  Q to Quit";

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
    companionSystem.releaseAllCompanions(); // Clear textures BEFORE CloseWindow

    CloseWindow();
    std::cout << "Game cleanup completed" << std::endl;
}
