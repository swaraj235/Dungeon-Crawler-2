#pragma once
#include "raylib.h"
#include <string>
#include <vector>

class Game;
class Player;

class HUD {
public:
    HUD(int width, int height);

    void draw(Game* game, Player* player);

    // Inventory navigation helpers
    int getSelectedInventoryItem() const { return selectedInventoryItem; }
    void setSelectedInventoryItem(int idx) { selectedInventoryItem = idx; }

private:
    int screenWidth;
    int screenHeight;

    Rectangle statsPanel;
    Rectangle inventoryPanel;

    int selectedInventoryItem;

    // Panel drawing
    void drawStatsPanel(Game* game, Player* player);
    void drawInventoryPanel(Player* player);
    void drawControlsPanel();
    void drawBuffIndicators(Player* player);
    void drawMiniMap(Game* game);
};
