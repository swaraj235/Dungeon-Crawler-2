#include "MapGenerator.h"
#include <algorithm>
#include <iostream>

MapGenerator::MapGenerator(int width, int height, int tSize)
    : mapWidth(width), mapHeight(height), tileSize(tSize), rng(std::random_device{}()) {

    tiles.resize(mapHeight, std::vector<Tile>(mapWidth));
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            tiles[y][x] = Tile(TileType::WALL, Vector2{(float)x * tSize, (float)y * tSize}, (float)tSize);
        }
    }
}

void MapGenerator::generateFloor(int floorNumber) {
    // Clear previous floor
    for (auto& row : tiles) {
        for (auto& tile : row) {
            tile.type = TileType::WALL;
        }
    }
    rooms.clear();
    decorativeElements.clear();
    decorativeTypes.clear();



    std::uniform_int_distribution<int> roomWidthDist(5, 12);
    std::uniform_int_distribution<int> roomHeightDist(5, 10);
    std::uniform_int_distribution<int> numRoomsDist(8 + floorNumber, 12 + floorNumber * 2);

    int numRooms = numRoomsDist(rng);

    for (int i = 0; i < numRooms; i++) {
        int w = roomWidthDist(rng);
        int h = roomHeightDist(rng);

        std::uniform_int_distribution<int> xDist(1, mapWidth - w - 2);
        std::uniform_int_distribution<int> yDist(1, mapHeight - h - 2);

        int x = xDist(rng);
        int y = yDist(rng);

        carveRoom(x, y, w, h);
        rooms.push_back({x, y, w, h});
    }

    // Add decorations ONLY in room corners, not center
    for (const auto& room : rooms) {
        // Add decorations in corners only
        std::vector<Vector2> corners = {
            {(float)(room.x + 1), (float)(room.y + 1)},  // Top-left
            {(float)(room.x + room.width - 2), (float)(room.y + 1)},  // Top-right
            {(float)(room.x + 1), (float)(room.y + room.height - 2)},  // Bottom-left
            {(float)(room.x + room.width - 2), (float)(room.y + room.height - 2)}  // Bottom-right
        };

        std::uniform_int_distribution<int> typeDist(0, 3);

        for (size_t i = 0; i < corners.size(); i++) {
            // Only place decoration 50% of the time
            if (rand() % 100 < 50) {
                Vector2 pos = {corners[i].x * tileSize, corners[i].y * tileSize};
                decorativeElements.push_back(pos);
                decorativeTypes.push_back(typeDist(rng));
            }
        }
    }

    connectRooms();

    std::cout << "Generated floor " << floorNumber << " with " << rooms.size() << " rooms" << std::endl;
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

            Color color = DARKGRAY;
            if (tile.type == TileType::FLOOR) color = Color{100, 100, 100, 255};
            else if (tile.type == TileType::DOOR) color = ORANGE;
            else if (tile.type == TileType::TRAP) color = RED;

            DrawRectangle((int)tile.position.x, (int)tile.position.y, tileSize, tileSize, color);
            DrawRectangleLines((int)tile.position.x, (int)tile.position.y, tileSize, tileSize, BLACK);
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

        switch (type) {
            case 0: // Water/Magical Lake
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
    Rectangle newBounds = {bounds.x + movement.x, bounds.y + movement.y, bounds.width, bounds.height};

    if (isWall(newBounds.x, newBounds.y) || isWall(newBounds.x + newBounds.width, newBounds.y) ||
        isWall(newBounds.x, newBounds.y + newBounds.height) || isWall(newBounds.x + newBounds.width, newBounds.y + newBounds.height)) {
        return {0, 0}; // No movement if collision
    }

    return movement;
}
