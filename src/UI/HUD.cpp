#include "HUD.h"
#include "Game.h"
#include "Player.h"
#include "ItemSystem.h"
#include <string>
#include <algorithm>

HUD::HUD(int width, int height)
    : screenWidth(width), screenHeight(height), selectedInventoryItem(0)
{
    statsPanel = {20.0f, 20.0f, 320.0f, 240.0f};
    inventoryPanel = {20.0f, 270.0f, 300.0f, 200.0f};
}

void HUD::draw(Game* game, Player* player) {
    if (!game || !player) return;

    drawStatsPanel(game, player);
    drawBuffIndicators(player);
    drawMiniMap(game);

    if (game->getInventoryOpen()) {
        drawInventoryPanel(player);

        if (IsKeyPressed(KEY_UP) && selectedInventoryItem > 0) {
            selectedInventoryItem--;
        }
        if (IsKeyPressed(KEY_DOWN) && selectedInventoryItem < (int)player->getInventory().size() - 1) {
            selectedInventoryItem++;
        }

        DrawText("Press I to Close | UP/DOWN Select | ENTER Use | ESC Close",
                20, screenHeight - 30, 10, Color{0, 255, 136, 255});
    } else {
        DrawText("Press I=Inventory  H/J/K/L=Potions  SPACE=Attack  1-4=Spells",
                20, screenHeight - 30, 10, Color{128, 128, 128, 255});
    }
}

void HUD::drawStatsPanel(Game* game, Player* player) {
    DrawRectangleRec(statsPanel, Fade(Color{0, 0, 0, 255}, 0.85f));
    DrawRectangleLinesEx(statsPanel, 2, Color{255, 255, 0, 255});

    int y = (int)statsPanel.y + 10;
    int lineHeight = 16;

    DrawText("=== PLAYER STATS ===", (int)statsPanel.x + 10, y, 12, Color{0, 255, 0, 255});
    y += lineHeight + 3;

    // HP text above the bar
    std::string hpStr = "HP: " + std::to_string(player->getHealth()) + "/" + std::to_string(player->getMaxHealth());
    DrawText(hpStr.c_str(), (int)statsPanel.x + 10, y, 15, Color{255, 255, 255, 255}); // larger font, 15px tall
    y += 22; // Add more than the font height

    // HP bar well under the text
    int barX = (int)statsPanel.x + 10;
    int barY = y;
    int barW = 200, barH = 15;
    DrawRectangle(barX, barY, barW, barH, Color{64, 64, 64, 255});
    float hpFrac = player->getMaxHealth() > 0 ? (float)player->getHealth() / (float)player->getMaxHealth() : 0.0f;
    hpFrac = std::max(0.0f, std::min(1.0f, hpFrac));
    int hpBarW = (int)(barW * hpFrac);
    DrawRectangle(barX, barY, hpBarW, barH, Color{0, 255, 0, 255});

    y += barH + 10; // Make even *more* space below for next stat

    std::string lvlStr = "Level: " + std::to_string(player->getLevel());
    DrawText(lvlStr.c_str(), (int)statsPanel.x + 10, y, 11, Color{255, 255, 0, 255});
    y += lineHeight;

    std::string expStr = "EXP: " + std::to_string(player->getExperience());
    DrawText(expStr.c_str(), (int)statsPanel.x + 10, y, 11, Color{255, 255, 0, 255});
    y += lineHeight;

    std::string scoreStr = "Score: " + std::to_string(game->getScore());
    DrawText(scoreStr.c_str(), (int)statsPanel.x + 10, y, 11, Color{0, 255, 0, 255});
    y += lineHeight;

    std::string floorStr = "Floor: " + std::to_string(game->getCurrentFloor());
    DrawText(floorStr.c_str(), (int)statsPanel.x + 10, y, 11, Color{0, 255, 255, 255});
    y += lineHeight + 5;

    DrawText("SPELLS:", (int)statsPanel.x + 10, y, 10, Color{102, 191, 255, 255});
    y += lineHeight;

    const auto& spells = player->getSpells();
    if (spells.empty()) {
        DrawText("  None unlocked", (int)statsPanel.x + 12, y, 10, Color{80, 80, 80, 255});
    } else {
        for (const auto& spell : spells) {
            std::string spellStr = "  [" + std::to_string(&spell - &spells[0] + 1) + "] " + spell.name;
            DrawText(spellStr.c_str(), (int)statsPanel.x + 12, y, 9, Color{0, 255, 0, 255});
            y += lineHeight - 3;
        }
    }
}

void HUD::drawInventoryPanel(Player* player) {
    if (!player) return;

    DrawRectangleRec(inventoryPanel, Fade(Color{0, 0, 0, 255}, 0.95f));
    DrawRectangleLinesEx(inventoryPanel, 2, Color{0, 255, 136, 255});

    int startY = (int)inventoryPanel.y + 12;
    int y = startY;
    int lineHeight = 16;

    DrawText("=== INVENTORY ===", (int)inventoryPanel.x + 10, y, 12, Color{0, 255, 136, 255});
    y += lineHeight + 4;

    const auto& inventory = player->getInventory();
    int invSize = (int)inventory.size();

    if (invSize == 0) {
        DrawText("Empty", (int)inventoryPanel.x + 20, y + 20, 12, Color{128, 128, 128, 255});
        return;
    }

    // Bounds check
    if (selectedInventoryItem >= invSize) selectedInventoryItem = invSize - 1;
    if (selectedInventoryItem < 0) selectedInventoryItem = 0;

    // Sliding window
    const int maxVisible = 11;
    int windowStart = 0;

    if (invSize > maxVisible) {
        windowStart = selectedInventoryItem - (maxVisible / 2);
        if (windowStart < 0) windowStart = 0;
        if (windowStart + maxVisible > invSize) windowStart = invSize - maxVisible;
    }

    for (int k = 0; k < maxVisible && (windowStart + k) < invSize; ++k) {
        int i = windowStart + k;
        Color col = Color{255, 255, 255, 255};
        std::string text = inventory[i].name + " x" + std::to_string(inventory[i].quantity);

        // Draw highlight rectangle ONLY for selected item
        if (i == selectedInventoryItem) {
            DrawRectangle((int)inventoryPanel.x + 8, y - 2, (int)inventoryPanel.width - 16, lineHeight,
                         Color{0, 100, 50, 150});
            DrawText(">>", (int)inventoryPanel.x + 12, y, 11, Color{0, 255, 0, 255});
            col = Color{0, 255, 0, 255}; // Green text for selected
        } else {
            col = Color{200, 200, 200, 255}; // Gray for unselected
        }

        // Item type coloring (only if not selected)
        if (i != selectedInventoryItem) {
            if (inventory[i].name.find("Potion") != std::string::npos) {
                col = Color{0, 200, 255, 255};
            } else if (inventory[i].name.find("Sword") != std::string::npos ||
                       inventory[i].name.find("Gauntlet") != std::string::npos) {
                col = Color{255, 100, 0, 255};
            } else if (inventory[i].name.find("Stone") != std::string::npos ||
                       inventory[i].name.find("Orb") != std::string::npos) {
                col = Color{255, 215, 0, 255};
            } else if (inventory[i].name.find("Necklace") != std::string::npos ||
                       inventory[i].name.find("Pendant") != std::string::npos) {
                col = Color{173, 216, 230, 255};
            }
        }

        DrawText(text.c_str(), (int)inventoryPanel.x + 30, y, 10, col);
        y += lineHeight;
    }

    // Scroll indicator
    if (invSize > maxVisible) {
        std::string scrollInfo = "(" + std::to_string(selectedInventoryItem + 1) + "/" + std::to_string(invSize) + ")";
        DrawText(scrollInfo.c_str(), (int)inventoryPanel.x + 10, (int)inventoryPanel.y + (int)inventoryPanel.height - 20, 10, Color{128, 128, 128, 255});
    }
}

void HUD::drawControlsPanel() {
    Rectangle panel = {20.0f, (float)(screenHeight - 210), 380.0f, 200.0f};
    DrawRectangleRec(panel, Fade(Color{0, 0, 0, 255}, 0.85f));
    DrawRectangleLinesEx(panel, 2, Color{255, 100, 100, 255});

    int y = screenHeight - 200;
    int lineHeight = 14;

    DrawText("=== CONTROLS ===", 30, y, 12, Color{255, 100, 100, 255});
    y += lineHeight + 5;

    DrawText("MOVE: WASD/Arrows | ATTACK: SPACE", 30, y, 10, Color{255, 255, 255, 255}); y += lineHeight;
    DrawText("INVENTORY: I | SELECT: UP/DOWN | USE: ENTER", 30, y, 10, Color{0, 255, 136, 255}); y += lineHeight;
    DrawText("QUICK POTIONS: H/J/K/L | SPELLS: 1-4", 30, y, 10, Color{102, 191, 255, 255}); y += lineHeight;
    DrawText("PAUSE: P | SAVE: Ctrl+S | QUIT: Q", 30, y, 10, Color{255, 255, 255, 255}); y += lineHeight;
}

void HUD::drawMiniMap(Game* game) {
    if (!game) return;

    auto* map = game->getMap();
    auto* playerObj = game->getPlayer();

    if (!map || !playerObj) return;

    int miniMapWidth = 150;
    int miniMapHeight = 150;
    int mapX = screenWidth - miniMapWidth - 10;
    int mapY = 10;

    DrawRectangle(mapX - 2, mapY - 2, miniMapWidth + 4, miniMapHeight + 4, Color{0, 0, 0, 255});
    DrawRectangle(mapX, mapY, miniMapWidth, miniMapHeight, Color{40, 40, 40, 200});

    float mapWidth = (float)(map->getMapWidth() * map->getTileSize());
    float mapHeight = (float)(map->getMapHeight() * map->getTileSize());
    if (mapWidth > 0 && mapHeight > 0) {
        float scale = (float)miniMapWidth / mapWidth;

        for (int y = 0; y < map->getMapHeight(); y += 2) {
            for (int x = 0; x < map->getMapWidth(); x += 2) {
                if (map->isWall((float)(x * map->getTileSize()), (float)(y * map->getTileSize()))) {
                    int pixelX = mapX + (int)(x * map->getTileSize() * scale);
                    int pixelY = mapY + (int)(y * map->getTileSize() * scale);
                    if (pixelX >= mapX && pixelX < mapX + miniMapWidth &&
                        pixelY >= mapY && pixelY < mapY + miniMapHeight) {
                        DrawPixel(pixelX, pixelY, Color{128, 128, 128, 255});
                    }
                }
            }
        }

        for (const auto& enemy : game->getEnemies()) {
            if (enemy->getIsAlive()) {
                Vector2 enemyPos = enemy->getPosition();
                int pixelX = mapX + (int)(enemyPos.x * scale);
                int pixelY = mapY + (int)(enemyPos.y * scale);
                if (pixelX >= mapX && pixelX < mapX + miniMapWidth &&
                    pixelY >= mapY && pixelY < mapY + miniMapHeight) {
                    DrawCircle(pixelX, pixelY, 2, Color{230, 41, 55, 255});
                }
            }
        }

        Vector2 playerPos = playerObj->getPosition();
        int playerX = mapX + (int)(playerPos.x * scale);
        int playerY = mapY + (int)(playerPos.y * scale);
        if (playerX >= mapX && playerX < mapX + miniMapWidth &&
            playerY >= mapY && playerY < mapY + miniMapHeight) {
            DrawCircle(playerX, playerY, 3, Color{0, 121, 241, 255});
            DrawCircle(playerX, playerY, 2, Color{255, 255, 255, 255});
        }
    }

    DrawRectangleLines(mapX, mapY, miniMapWidth, miniMapHeight, Color{0, 255, 200, 255});
    DrawText("MAP", mapX + 5, mapY + miniMapHeight + 5, 10, Color{255, 255, 255, 255});
}

void HUD::drawBuffIndicators(Player* player) {
    if (!player) return;

    int y = screenHeight - 100;
}
