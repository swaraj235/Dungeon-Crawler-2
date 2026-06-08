#include "HUD.h"
#include "Game.h"
#include "Player.h"
#include "ItemSystem.h"
#include <string>
#include <algorithm>
#include <cmath>

// ─── Layout Constants ───────────────────────────────────────────
static constexpr int MARGIN = 15;
static constexpr int PANEL_RADIUS = 6;

// Panel colors
static const Color PANEL_BG       = {10, 12, 18, 200};
static const Color PANEL_BORDER   = {60, 70, 90, 180};
static const Color ACCENT_GREEN   = {0, 230, 118, 255};
static const Color ACCENT_CYAN    = {0, 200, 220, 255};
static const Color ACCENT_YELLOW  = {255, 214, 10, 255};
static const Color ACCENT_RED     = {255, 69, 58, 255};
static const Color TEXT_PRIMARY   = {230, 230, 240, 255};
static const Color TEXT_SECONDARY = {140, 145, 160, 255};
static const Color TEXT_DIM       = {90, 95, 110, 255};
static const Color HP_BAR_BG     = {30, 32, 40, 255};
static const Color HP_BAR_FILL   = {0, 210, 80, 255};
static const Color HP_BAR_LOW    = {255, 69, 58, 255};
static const Color HP_BAR_MED    = {255, 159, 10, 255};
static const Color EXP_BAR_FILL  = {94, 92, 230, 255};

HUD::HUD(int width, int height)
    : screenWidth(width), screenHeight(height), selectedInventoryItem(0)
{
    statsPanel = {(float)MARGIN, (float)MARGIN, 210.0f, 158.0f};
    inventoryPanel = {(float)MARGIN, 185.0f, 280.0f, 220.0f};
}

// ─── Helper: Draw a rounded-corner panel ────────────────────────
static void drawPanel(Rectangle rect, Color bg, Color border) {
    DrawRectangleRounded(rect, 0.1f, 8, bg);
    DrawRectangleRoundedLinesEx(rect, 0.1f, 8, 1.0f, border);
}

void HUD::draw(Game* game, Player* player) {
    if (!game || !player) return;

    drawStatsPanel(game, player);
    drawMiniMap(game);
    drawBuffIndicators(player);
    drawPotionBar(player);

    if (game->getInventoryOpen()) {
        drawInventoryPanel(player);

        if (IsKeyPressed(KEY_UP) && selectedInventoryItem > 0) {
            selectedInventoryItem--;
        }
        if (IsKeyPressed(KEY_DOWN) && selectedInventoryItem < (int)player->getInventory().size() - 1) {
            selectedInventoryItem++;
        }

        DrawText("I=Close  \xe2\x86\x91\xe2\x86\x93=Select  ENTER=Use",
                MARGIN, screenHeight - 28, 10, ACCENT_GREEN);
    } else {
        // Subtle controls hint at bottom-center
        const char* hint = "WASD=Move  SPACE=Attack  I=Inventory  1-4=Spells  H/J/K/L=Potions";
        int hintW = MeasureText(hint, 10);
        DrawText(hint, (screenWidth - hintW) / 2, screenHeight - 24, 10, TEXT_DIM);
    }
}

// ─── Stats Panel (Top-Left, compact) ────────────────────────────
void HUD::drawStatsPanel(Game* game, Player* player) {
    drawPanel(statsPanel, PANEL_BG, PANEL_BORDER);

    int px = (int)statsPanel.x + 12;
    int py = (int)statsPanel.y + 10;

    // ── Player name + level on same line
    std::string nameStr = player->playerName.empty() ? "Adventurer" : player->playerName;
    DrawText(nameStr.c_str(), px, py, 12, TEXT_PRIMARY);

    std::string lvlStr = "Lv." + std::to_string(player->getLevel());
    int lvlW = MeasureText(lvlStr.c_str(), 12);
    DrawText(lvlStr.c_str(), (int)(statsPanel.x + statsPanel.width) - lvlW - 12, py, 12, ACCENT_YELLOW);
    py += 20;

    // ── HP bar
    float hpFrac = player->getMaxHealth() > 0
        ? (float)player->getHealth() / (float)player->getMaxHealth() : 0.0f;
    hpFrac = std::max(0.0f, std::min(1.0f, hpFrac));

    Color hpColor = hpFrac > 0.5f ? HP_BAR_FILL : (hpFrac > 0.25f ? HP_BAR_MED : HP_BAR_LOW);

    int barW = (int)statsPanel.width - 24;
    int barH = 10;

    DrawText("HP", px, py, 9, TEXT_SECONDARY);
    std::string hpText = std::to_string(player->getHealth()) + "/" + std::to_string(player->getMaxHealth());
    int hpTxtW = MeasureText(hpText.c_str(), 9);
    DrawText(hpText.c_str(), px + barW - hpTxtW, py, 9, TEXT_PRIMARY);
    py += 12;

    DrawRectangle(px, py, barW, barH, HP_BAR_BG);
    DrawRectangle(px, py, (int)(barW * hpFrac), barH, hpColor);
    py += barH + 8;

    // ── EXP bar
    int expForNext = 100; // simplified — Config::EXP_FOR_LEVEL_2 * pow(scaling, level-1)
    float expFrac = expForNext > 0
        ? (float)player->getExperience() / (float)expForNext : 0.0f;
    expFrac = std::max(0.0f, std::min(1.0f, expFrac));

    DrawText("EXP", px, py, 9, TEXT_SECONDARY);
    std::string expText = std::to_string(player->getExperience()) + "/" + std::to_string(expForNext);
    int expTxtW = MeasureText(expText.c_str(), 9);
    DrawText(expText.c_str(), px + barW - expTxtW, py, 9, TEXT_PRIMARY);
    py += 12;

    DrawRectangle(px, py, barW, 6, HP_BAR_BG);
    DrawRectangle(px, py, (int)(barW * expFrac), 6, EXP_BAR_FILL);
    py += 14;

    // ── Floor + Score row
    DrawRectangle(px, py, barW, 1, Color{50, 55, 70, 150});  // subtle divider
    py += 6;

    std::string floorStr = "Floor " + std::to_string(game->getCurrentFloor());
    DrawText(floorStr.c_str(), px, py, 10, ACCENT_CYAN);

    std::string scoreStr = std::to_string(game->getScore()) + " pts";
    int scoreW = MeasureText(scoreStr.c_str(), 10);
    DrawText(scoreStr.c_str(), px + barW - scoreW, py, 10, ACCENT_GREEN);
    py += 18;

    // ── Spells row (compact icons)
    const auto& spells = player->getSpells();
    if (!spells.empty()) {
        DrawText("Spells:", px, py, 9, TEXT_SECONDARY);
        int sx = px + 48;
        for (size_t i = 0; i < spells.size() && i < 4; i++) {
            std::string tag = "[" + std::to_string(i + 1) + "]";
            DrawText(tag.c_str(), sx, py, 9, ACCENT_CYAN);
            sx += MeasureText(tag.c_str(), 9) + 6;
        }
    } else {
        DrawText("No spells", px, py, 9, TEXT_DIM);
    }
}

// ─── Inventory Panel (Below stats, shown on I key) ──────────────
void HUD::drawInventoryPanel(Player* player) {
    if (!player) return;

    drawPanel(inventoryPanel, PANEL_BG, PANEL_BORDER);

    int px = (int)inventoryPanel.x + 12;
    int py = (int)inventoryPanel.y + 10;
    int lineH = 16;

    DrawText("INVENTORY", px, py, 11, ACCENT_GREEN);
    py += lineH + 4;

    const auto& inventory = player->getInventory();
    int invSize = (int)inventory.size();

    if (invSize == 0) {
        DrawText("Empty", px + 10, py + 20, 11, TEXT_DIM);
        return;
    }

    if (selectedInventoryItem >= invSize) selectedInventoryItem = invSize - 1;
    if (selectedInventoryItem < 0) selectedInventoryItem = 0;

    const int maxVisible = 11;
    int windowStart = 0;

    if (invSize > maxVisible) {
        windowStart = selectedInventoryItem - (maxVisible / 2);
        if (windowStart < 0) windowStart = 0;
        if (windowStart + maxVisible > invSize) windowStart = invSize - maxVisible;
    }

    for (int k = 0; k < maxVisible && (windowStart + k) < invSize; ++k) {
        int i = windowStart + k;
        Color col = TEXT_PRIMARY;
        std::string text = inventory[i].name + " x" + std::to_string(inventory[i].quantity);

        if (i == selectedInventoryItem) {
            DrawRectangleRounded(
                {(float)(px - 2), (float)(py - 2), inventoryPanel.width - 20, (float)lineH},
                0.3f, 4, Color{0, 230, 118, 30});
            DrawText("\xe2\x96\xb6", px, py, 10, ACCENT_GREEN);
            col = ACCENT_GREEN;
        } else {
            // Item type coloring
            if (inventory[i].name.find("Potion") != std::string::npos) {
                col = ACCENT_CYAN;
            } else if (inventory[i].name.find("Sword") != std::string::npos ||
                       inventory[i].name.find("Gauntlet") != std::string::npos) {
                col = {255, 149, 0, 255};
            } else if (inventory[i].name.find("Stone") != std::string::npos ||
                       inventory[i].name.find("Orb") != std::string::npos) {
                col = ACCENT_YELLOW;
            } else if (inventory[i].name.find("Necklace") != std::string::npos ||
                       inventory[i].name.find("Pendant") != std::string::npos) {
                col = {175, 130, 255, 255};
            }
        }

        DrawText(text.c_str(), px + 18, py, 10, col);
        py += lineH;
    }

    if (invSize > maxVisible) {
        std::string scrollInfo = std::to_string(selectedInventoryItem + 1) + "/" + std::to_string(invSize);
        int scrollW = MeasureText(scrollInfo.c_str(), 9);
        DrawText(scrollInfo.c_str(),
                (int)(inventoryPanel.x + inventoryPanel.width) - scrollW - 12,
                (int)(inventoryPanel.y + inventoryPanel.height) - 16, 9, TEXT_DIM);
    }
}

void HUD::drawControlsPanel() {
    // Controls are now shown as a single centered line at the bottom — see draw()
}

// ─── Minimap (Top-Right) ────────────────────────────────────────
void HUD::drawMiniMap(Game* game) {
    if (!game) return;

    auto* map = game->getMap();
    auto* playerObj = game->getPlayer();
    if (!map || !playerObj) return;

    int miniW = 170;
    int miniH = 170;
    int mapX = screenWidth - miniW - MARGIN;
    int mapY = MARGIN;

    Rectangle miniRect = {(float)mapX, (float)mapY, (float)miniW, (float)miniH};
    drawPanel(miniRect, PANEL_BG, PANEL_BORDER);

    // Inner area (2px inset)
    int innerX = mapX + 4;
    int innerY = mapY + 4;
    int innerW = miniW - 8;
    int innerH = miniH - 8;

    float mapWidth  = (float)(map->getMapWidth() * map->getTileSize());
    float mapHeight = (float)(map->getMapHeight() * map->getTileSize());
    if (mapWidth <= 0 || mapHeight <= 0) return;

    float scaleX = (float)innerW / mapWidth;
    float scaleY = (float)innerH / mapHeight;
    float scale  = std::min(scaleX, scaleY);

    // Draw walls
    for (int y = 0; y < map->getMapHeight(); y += 2) {
        for (int x = 0; x < map->getMapWidth(); x += 2) {
            if (map->isWall((float)(x * map->getTileSize()), (float)(y * map->getTileSize()))) {
                int px = innerX + (int)(x * map->getTileSize() * scale);
                int py = innerY + (int)(y * map->getTileSize() * scale);
                if (px >= innerX && px < innerX + innerW &&
                    py >= innerY && py < innerY + innerH) {
                    DrawPixel(px, py, Color{90, 100, 120, 255});
                }
            }
        }
    }

    // Draw enemies as small red dots
    for (const auto& enemy : game->getEnemies()) {
        if (enemy->getIsAlive()) {
            Vector2 ePos = enemy->getPosition();
            int ex = innerX + (int)(ePos.x * scale);
            int ey = innerY + (int)(ePos.y * scale);
            if (ex >= innerX && ex < innerX + innerW &&
                ey >= innerY && ey < innerY + innerH) {
                DrawCircle(ex, ey, 2, ACCENT_RED);
            }
        }
    }

    // Draw player as bright dot with glow
    Vector2 pPos = playerObj->getPosition();
    int plX = innerX + (int)(pPos.x * scale);
    int plY = innerY + (int)(pPos.y * scale);
    if (plX >= innerX && plX < innerX + innerW &&
        plY >= innerY && plY < innerY + innerH) {
        DrawCircle(plX, plY, 4, Color{0, 180, 255, 80});  // glow
        DrawCircle(plX, plY, 2, Color{100, 200, 255, 255});
    }
}

// ─── Buff Indicators (Bottom-Left) ──────────────────────────────
void HUD::drawBuffIndicators(Player* player) {
    if (!player) return;

    int bx = MARGIN;
    int by = screenHeight - 70;
    int iconSize = 32;
    int gap = 8;

    struct BuffInfo {
        const char* label;
        const char* icon;
        float remaining;
        Color color;
    };

    BuffInfo buffs[] = {
        {"SPD",  "S", player->getSpeedBuffTime(),    {0, 230, 118, 255}},
        {"RAGE", "R", player->getRageBuffTime(),      {255, 69, 58, 255}},
        {"HIDE", "H", player->getIsStealthed() ? 1.0f : 0.0f, {160, 130, 255, 255}},
    };

    for (auto& b : buffs) {
        if (b.remaining > 0.01f) {
            // Background
            Rectangle r = {(float)bx, (float)by, (float)iconSize, (float)iconSize};
            DrawRectangleRounded(r, 0.25f, 4, Color{b.color.r, b.color.g, b.color.b, 40});
            DrawRectangleRoundedLinesEx(r, 0.25f, 4, 1.0f,
                Color{b.color.r, b.color.g, b.color.b, 150});

            // Icon letter
            int tw = MeasureText(b.icon, 14);
            DrawText(b.icon, bx + (iconSize - tw) / 2, by + 4, 14, b.color);

            // Timer or label
            std::string timerStr = b.remaining > 99 ? b.label : std::to_string((int)b.remaining) + "s";
            int lw = MeasureText(timerStr.c_str(), 8);
            DrawText(timerStr.c_str(), bx + (iconSize - lw) / 2, by + 22, 8, TEXT_SECONDARY);

            bx += iconSize + gap;
        }
    }
}

// ─── Quick Potion Bar (Bottom-Right) ────────────────────────────
void HUD::drawPotionBar(Player* player) {
    if (!player) return;

    struct PotionInfo {
        const char* key;
        const char* name;
        const char* shortName;
        Color color;
    };

    PotionInfo potions[] = {
        {"H", "Health Potion",  "HP",  {255, 69, 58, 255}},
        {"J", "Speed Potion",   "SPD", {0, 230, 118, 255}},
        {"K", "Stealth Potion", "STL", {160, 130, 255, 255}},
        {"L", "Rage Potion",    "RGE", {255, 159, 10, 255}},
    };

    int slotW = 44;
    int slotH = 38;
    int gap = 4;
    int totalW = 4 * slotW + 3 * gap;
    int bx = screenWidth - totalW - MARGIN;
    int by = screenHeight - slotH - MARGIN;

    for (int i = 0; i < 4; i++) {
        int sx = bx + i * (slotW + gap);

        // Count this potion in inventory
        int count = 0;
        for (const auto& item : player->getInventory()) {
            if (item.name == potions[i].name) {
                count = item.quantity;
                break;
            }
        }

        Color bgCol = count > 0
            ? Color{potions[i].color.r, potions[i].color.g, potions[i].color.b, 25}
            : Color{30, 32, 40, 180};
        Color borderCol = count > 0
            ? Color{potions[i].color.r, potions[i].color.g, potions[i].color.b, 120}
            : Color{50, 55, 70, 120};

        Rectangle slot = {(float)sx, (float)by, (float)slotW, (float)slotH};
        DrawRectangleRounded(slot, 0.2f, 4, bgCol);
        DrawRectangleRoundedLinesEx(slot, 0.2f, 4, 1.0f, borderCol);

        // Key binding label
        Color keyCol = count > 0 ? potions[i].color : TEXT_DIM;
        int kw = MeasureText(potions[i].key, 12);
        DrawText(potions[i].key, sx + (slotW - kw) / 2, by + 4, 12, keyCol);

        // Count
        std::string countStr = "x" + std::to_string(count);
        int cw = MeasureText(countStr.c_str(), 9);
        Color cCol = count > 0 ? TEXT_PRIMARY : TEXT_DIM;
        DrawText(countStr.c_str(), sx + (slotW - cw) / 2, by + 22, 9, cCol);
    }
}
