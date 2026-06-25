#include "MapGenerator.h"
#include <algorithm>
#include <iostream>

MapGenerator::MapGenerator(int width, int height, int tSize)
    : mapWidth(width), mapHeight(height), tileSize(tSize),
      currentFloorNumber(1), currentTheme(FloorTheme::STONE_DUNGEON),
      throneRoomCenter({0,0}), rng(std::random_device{}()) {
    tiles.resize(mapHeight, std::vector<Tile>(mapWidth));
    for (int y = 0; y < mapHeight; y++)
        for (int x = 0; x < mapWidth; x++)
            tiles[y][x] = Tile(TileType::WALL, Vector2{(float)x*tSize,(float)y*tSize}, (float)tSize);
}

void MapGenerator::setTheme(int floorNum) {
    currentFloorNumber = floorNum;
    if      (floorNum == 1) currentTheme = FloorTheme::STONE_DUNGEON;
    else if (floorNum == 2) currentTheme = FloorTheme::CATACOMBS;
    else if (floorNum == 3) currentTheme = FloorTheme::SHADOW_PALACE;
    else                    currentTheme = FloorTheme::FROZEN_WASTES;
}

void MapGenerator::generateBossRoom() {
    // Place a large throne room in the lower-right quadrant
    int bossW = 16, bossH = 12;
    int bossX = mapWidth - bossW - 4;
    int bossY = mapHeight - bossH - 4;
    carveRoom(bossX, bossY, bossW, bossH);
    rooms.push_back({bossX, bossY, bossW, bossH});
    throneRoomCenter = {
        (float)(bossX + bossW/2) * tileSize,
        (float)(bossY + bossH/2) * tileSize
    };
    // Connect boss room to last normal room
    if (rooms.size() >= 2) {
        auto& prev = rooms[rooms.size()-2];
        carvePath(prev.x + prev.width/2, prev.y + prev.height/2,
                  bossX + bossW/2, bossY + bossH/2);
    }
}

void MapGenerator::generateFloor(int floorNumber) {
    for (auto& row : tiles)
        for (auto& tile : row)
            tile.type = TileType::WALL;
    rooms.clear();
    decorativeElements.clear();
    decorativeTypes.clear();
    throneRoomCenter = {0, 0};

    setTheme(floorNumber);

    std::uniform_int_distribution<int> roomWidthDist(5, 12);
    std::uniform_int_distribution<int> roomHeightDist(5, 10);
    std::uniform_int_distribution<int> numRoomsDist(8 + floorNumber, 12 + floorNumber * 2);
    int numRooms = numRoomsDist(rng);

    for (int i = 0; i < numRooms; i++) {
        int w = roomWidthDist(rng), h = roomHeightDist(rng);
        std::uniform_int_distribution<int> xDist(1, mapWidth - w - 2);
        std::uniform_int_distribution<int> yDist(1, mapHeight - h - 2);
        int x = xDist(rng), y = yDist(rng);
        carveRoom(x, y, w, h);
        rooms.push_back({x, y, w, h});
    }

    // Floor 3: guaranteed throne room for Shadow Paladin boss
    if (floorNumber == 3) {
        generateBossRoom();
    }

    connectRooms();

    // Decorations: placed in room corners, theme-specific
    std::uniform_int_distribution<int> typeDist(0, 3);
    for (const auto& room : rooms) {
        std::vector<Vector2> corners = {
            {(float)(room.x+1),              (float)(room.y+1)},
            {(float)(room.x+room.width-2),   (float)(room.y+1)},
            {(float)(room.x+1),              (float)(room.y+room.height-2)},
            {(float)(room.x+room.width-2),   (float)(room.y+room.height-2)}
        };
        for (auto& c : corners) {
            if (rand() % 100 < 50) {
                decorativeElements.push_back({c.x * tileSize, c.y * tileSize});
                decorativeTypes.push_back(typeDist(rng));
            }
        }
    }

    std::cout << "Generated floor " << floorNumber << " with " << rooms.size() << " rooms (theme " << (int)currentTheme << ")" << std::endl;
}

void MapGenerator::carveRoom(int x, int y, int w, int h) {
    for (int ty = y; ty < y + h; ty++) {
        for (int tx = x; tx < x + w; tx++) {
            if (ty >= 0 && ty < mapHeight && tx >= 0 && tx < mapWidth) {
                tiles[ty][tx].type = TileType::FLOOR;
            }
        }
    }
}

void MapGenerator::connectRooms() {
    for (size_t i = 1; i < rooms.size(); i++) {
        int prevX = rooms[i - 1].x + rooms[i - 1].width / 2;
        int prevY = rooms[i - 1].y + rooms[i - 1].height / 2;

        int currX = rooms[i].x + rooms[i].width / 2;
        int currY = rooms[i].y + rooms[i].height / 2;

        // Make corridors wider (3 tiles instead of 1)
        carvePath(prevX, prevY, currX, currY);
    }
}

void MapGenerator::carvePath(int x1, int y1, int x2, int y2) {
    int x = x1, y = y1;

    // Horizontal path with width
    while (x != x2) {
        if (x < x2) x++;
        else x--;

        for (int offset = -1; offset <= 1; offset++) {
            int ty = y + offset;
            if (ty >= 0 && ty < mapHeight && x >= 0 && x < mapWidth) {
                tiles[ty][x].type = TileType::FLOOR;
            }
        }
    }

    // Vertical path with width
    while (y != y2) {
        if (y < y2) y++;
        else y--;

        for (int offset = -1; offset <= 1; offset++) {
            int tx = x + offset;
            if (y >= 0 && y < mapHeight && tx >= 0 && tx < mapWidth) {
                tiles[y][tx].type = TileType::FLOOR;
            }
        }
    }
}

void MapGenerator::draw() {
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            Tile& tile = tiles[y][x];

            // 1. Draw Floor, Door, or Trap
            if (tile.type == TileType::FLOOR || tile.type == TileType::DOOR || tile.type == TileType::TRAP) {
                Color floorCol = {75, 71, 65, 255};
                Color floorHighlight = {60, 56, 50, 255};
                Color floorPebble = {85, 80, 75, 255};
                Color floorGrid = {65, 60, 55, 100};

                if (currentTheme == FloorTheme::CATACOMBS) {
                    floorCol = {55, 50, 60, 255};
                    floorHighlight = {45, 40, 50, 255};
                    floorPebble = {65, 60, 70, 255};
                    floorGrid = {50, 45, 55, 100};
                } else if (currentTheme == FloorTheme::SHADOW_PALACE) {
                    floorCol = {20, 15, 35, 255};
                    floorHighlight = {15, 10, 25, 255};
                    floorPebble = {30, 20, 45, 255};
                    floorGrid = {25, 20, 40, 100};
                } else if (currentTheme == FloorTheme::FROZEN_WASTES) {
                    floorCol = {140, 180, 210, 255};
                    floorHighlight = {120, 160, 195, 255};
                    floorPebble = {160, 200, 225, 255};
                    floorGrid = {100, 140, 175, 120};
                }

                if (tile.type == TileType::DOOR) floorCol = Color{130, 90, 40, 255};
                if (tile.type == TileType::TRAP) floorCol = Color{120, 40, 40, 255};

                DrawRectangle((int)tile.position.x, (int)tile.position.y, tileSize, tileSize, floorCol);
                
                if (tile.type == TileType::FLOOR) {
                    if ((x*13 + y*17) % 5 == 0) DrawRectangle((int)tile.position.x + 4, (int)tile.position.y + 6, 2, 2, floorHighlight);
                    if ((x*23 + y*11) % 7 == 0) DrawRectangle((int)tile.position.x + 22, (int)tile.position.y + 18, 3, 2, floorPebble);
                    DrawRectangleLinesEx({tile.position.x, tile.position.y, (float)tileSize, (float)tileSize}, 1.0f, floorGrid);
                }
            } 
            // 2. Draw Wall
            else if (tile.type == TileType::WALL) {
                // Determine if this wall has a floor tile below it (to draw the front face)
                bool isExposedFace = (y + 1 < mapHeight && tiles[y + 1][x].type != TileType::WALL);
                
                Color topColor = {38, 36, 42, 255};
                Color topHighlight = {28, 26, 32, 255};
                Color frontColor = {20, 18, 24, 255};
                Color edge1 = {50, 48, 58, 255};
                Color edge2 = {25, 23, 28, 255};

                if (currentTheme == FloorTheme::CATACOMBS) {
                    topColor = {30, 25, 40, 255};
                    topHighlight = {20, 15, 30, 255};
                    frontColor = {15, 10, 25, 255};
                    edge1 = {40, 35, 50, 255};
                    edge2 = {20, 15, 30, 255};
                } else if (currentTheme == FloorTheme::SHADOW_PALACE) {
                    topColor = {10, 5, 20, 255};
                    topHighlight = {5, 0, 10, 255};
                    frontColor = {5, 2, 10, 255};
                    edge1 = {25, 15, 40, 255};
                    edge2 = {10, 5, 20, 255};
                } else if (currentTheme == FloorTheme::FROZEN_WASTES) {
                    topColor = {90, 140, 175, 255};
                    topHighlight = {70, 120, 158, 255};
                    frontColor = {50, 90, 130, 255};
                    edge1 = {120, 165, 195, 255};
                    edge2 = {60, 110, 148, 255};
                }

                DrawRectangle((int)tile.position.x, (int)tile.position.y, tileSize, tileSize, topColor);
                DrawLineV({tile.position.x, tile.position.y + tileSize/2.0f}, {tile.position.x + tileSize, tile.position.y + tileSize/2.0f}, topHighlight);
                
                if ((x+y) % 2 == 0) {
                    DrawLineV({tile.position.x + tileSize/2.0f, tile.position.y}, {tile.position.x + tileSize/2.0f, tile.position.y + tileSize/2.0f}, topHighlight);
                } else {
                    DrawLineV({tile.position.x + tileSize/2.0f, tile.position.y + tileSize/2.0f}, {tile.position.x + tileSize/2.0f, tile.position.y + tileSize}, topHighlight);
                }
                
                if (isExposedFace) {
                    DrawRectangle((int)tile.position.x, (int)tile.position.y + tileSize - 8, tileSize, 8, frontColor);
                } else {
                    DrawLineV({tile.position.x, tile.position.y}, {tile.position.x, tile.position.y + (float)tileSize}, edge1);
                    DrawLineV({tile.position.x + (float)tileSize, tile.position.y}, {tile.position.x + (float)tileSize, tile.position.y + (float)tileSize}, edge2);
                }
            }
        }
    }
    // Draw decorations
    drawDecorations();
}

void MapGenerator::drawDecorations() {
    for (size_t i = 0; i < decorativeElements.size(); i++) {
        Vector2 pos = decorativeElements[i];
        int type = decorativeTypes[i];
        float x = pos.x + tileSize / 2;
        float y = pos.y + tileSize / 2;

        if (currentTheme == FloorTheme::STONE_DUNGEON) {
            switch (type) {
                case 0: // Water
                    DrawCircleV({x, y}, 10, Color{0, 150, 200, 180});
                    DrawCircleV({x, y}, 8, SKYBLUE);
                    DrawCircleLines((int)x, (int)y, 10, BLUE);
                    break;
                case 1: // Magic Stone
                    DrawRectangle((int)x - 6, (int)y - 6, 12, 12, Color{150, 100, 255, 200});
                    DrawRectangleLines((int)x - 6, (int)y - 6, 12, 12, Color{200, 150, 255, 255});
                    break;
                case 2: // Torch
                    DrawCircleV({x, y - 5}, 4, YELLOW);
                    DrawRectangle((int)x - 2, (int)y + 5, 4, 8, Color{100, 50, 0, 255});
                    DrawCircleV({x, y - 5}, 3, Color{255, 200, 0, 150});
                    break;
                case 3: // Rune
                    DrawRectangle((int)x - 8, (int)y - 8, 16, 16, Fade(PURPLE, 0.3f));
                    DrawText("*", (int)x - 3, (int)y - 5, 14, PURPLE);
                    DrawRectangleLines((int)x - 8, (int)y - 8, 16, 16, PURPLE);
                    break;
            }
        } else if (currentTheme == FloorTheme::CATACOMBS) {
            switch (type) {
                case 0: // Skull
                    DrawCircleV({x, y}, 6, LIGHTGRAY);
                    DrawCircleV({x-2, y}, 2, BLACK);
                    DrawCircleV({x+2, y}, 2, BLACK);
                    DrawRectangle((int)x-3, (int)y+4, 6, 4, LIGHTGRAY);
                    break;
                case 1: // Bone candle
                    DrawRectangle((int)x-2, (int)y-4, 4, 12, RAYWHITE);
                    DrawCircleV({x, y-6}, 3, Color{100, 255, 100, 200}); // eerie green flame
                    break;
                case 2: // Ancient Coffin
                    DrawRectangle((int)x-8, (int)y-12, 16, 24, Color{40, 30, 30, 255});
                    DrawRectangleLines((int)x-8, (int)y-12, 16, 24, Color{60, 50, 50, 255});
                    DrawLine((int)x-4, (int)y-8, (int)x+4, (int)y+8, Color{20, 10, 10, 255});
                    break;
                case 3: // Cobweb
                    DrawLine((int)x-8, (int)y-8, (int)x+8, (int)y+8, Fade(RAYWHITE, 0.5f));
                    DrawLine((int)x+8, (int)y-8, (int)x-8, (int)y+8, Fade(RAYWHITE, 0.5f));
                    DrawLine((int)x, (int)y-10, (int)x, (int)y+10, Fade(RAYWHITE, 0.5f));
                    DrawLine((int)x-10, (int)y, (int)x+10, (int)y, Fade(RAYWHITE, 0.5f));
                    break;
            }
        } else if (currentTheme == FloorTheme::SHADOW_PALACE) {
            switch (type) {
                case 0: // Shadow Crystal
                    DrawTriangle({x, y-10}, {x-6, y+8}, {x+6, y+8}, PURPLE);
                    DrawTriangle({x, y+14}, {x+6, y+8}, {x-6, y+8}, DARKPURPLE);
                    break;
                case 1: // Dark Rune
                    DrawCircleV({x, y}, 12, Color{20, 0, 30, 200});
                    DrawCircleLines((int)x, (int)y, 12, MAGENTA);
                    DrawText("?", (int)x - 4, (int)y - 6, 14, MAGENTA);
                    break;
                case 2: // Void flame
                    DrawCircleV({x, y}, 8, Color{50, 0, 100, 255});
                    DrawCircleV({x, y-4}, 5, MAGENTA);
                    DrawCircleV({x, y-8}, 3, WHITE);
                    break;
                case 3: // Broken Pillar
                    DrawRectangle((int)x-6, (int)y-10, 12, 20, Color{30, 30, 40, 255});
                    DrawLine((int)x-6, (int)y, (int)x+2, (int)y+4, BLACK);
                    DrawRectangleLines((int)x-6, (int)y-10, 12, 20, BLACK);
                    break;
            }
        }
    }

    if (currentTheme == FloorTheme::SHADOW_PALACE && throneRoomCenter.x != 0) {
        float tx = throneRoomCenter.x;
        float ty = throneRoomCenter.y;
        
        // Base red carpet
        DrawRectangle((int)tx - 32, (int)ty - 16, 64, 48, Color{100, 10, 20, 255});
        DrawRectangleLines((int)tx - 32, (int)ty - 16, 64, 48, Color{150, 20, 40, 255});
        
        // Throne back
        DrawRectangle((int)tx - 16, (int)ty - 40, 32, 40, Color{20, 10, 30, 255});
        DrawRectangleLines((int)tx - 16, (int)ty - 40, 32, 40, GOLD);
        
        // Throne seat
        DrawRectangle((int)tx - 16, (int)ty, 32, 16, Color{40, 20, 50, 255});
        DrawRectangleLines((int)tx - 16, (int)ty, 32, 16, GOLD);
        
        // Glowing magic circle under throne
        DrawCircleLines((int)tx, (int)ty, 40, Fade(MAGENTA, 0.6f));
        DrawCircleLines((int)tx, (int)ty, 45, Fade(PURPLE, 0.3f));
    }
}

bool MapGenerator::isWall(float x, float y) const {
    int gridX = (int)(x / tileSize);
    int gridY = (int)(y / tileSize);

    if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) {
        return true;
    }

    return tiles[gridY][gridX].type == TileType::WALL;
}

Vector2 MapGenerator::getRandomSpawnPosition() {
    if (rooms.empty()) return {100, 100};

    std::uniform_int_distribution<size_t> roomDist(0, rooms.size() - 1);
    Room& room = rooms[roomDist(rng)];

    std::uniform_int_distribution<int> xDist(room.x + 1, room.x + room.width - 2);
    std::uniform_int_distribution<int> yDist(room.y + 1, room.y + room.height - 2);

    return Vector2{(float)xDist(rng) * tileSize, (float)yDist(rng) * tileSize};
}

std::vector<Vector2> MapGenerator::getSpawnPositions(int count) {
    std::vector<Vector2> positions;

    for (int i = 0; i < count; i++) {
        positions.push_back(getRandomSpawnPosition());
    }

    return positions;
}

Vector2 MapGenerator::resolveCollision(Rectangle bounds, Vector2 movement) {
    Vector2 result = movement;

    // Shrink bounds very slightly so being perfectly flush with a wall doesn't count as overlapping it
    float shrink = 0.5f;
    Rectangle b = { bounds.x + shrink, bounds.y + shrink, bounds.width - 2*shrink, bounds.height - 2*shrink };

    // Try X axis alone
    Rectangle testX = {b.x + movement.x, b.y, b.width, b.height};
    bool hitX = isWall(testX.x, testX.y) || isWall(testX.x + testX.width, testX.y) ||
                isWall(testX.x, testX.y + testX.height) || isWall(testX.x + testX.width, testX.y + testX.height);
    if (hitX) result.x = 0;

    // Try Y axis alone
    Rectangle testY = {b.x, b.y + movement.y, b.width, b.height};
    bool hitY = isWall(testY.x, testY.y) || isWall(testY.x + testY.width, testY.y) ||
                isWall(testY.x, testY.y + testY.height) || isWall(testY.x + testY.width, testY.y + testY.height);
    if (hitY) result.y = 0;

    // If neither X nor Y hit independently, but combined diagonal hits a protruding corner:
    if (!hitX && !hitY) {
        Rectangle testXY = {b.x + movement.x, b.y + movement.y, b.width, b.height};
        if (isWall(testXY.x, testXY.y) || isWall(testXY.x + testXY.width, testXY.y) ||
            isWall(testXY.x, testXY.y + testXY.height) || isWall(testXY.x + testXY.width, testXY.y + testXY.height)) {
            // Cancel the smaller movement component to slide around the corner
            if (std::abs(movement.x) > std::abs(movement.y)) {
                result.y = 0;
            } else {
                result.x = 0;
            }
        }
    }

    return result;
}
