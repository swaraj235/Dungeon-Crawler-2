#pragma once
#include "raylib.h"
#include <vector>
#include <random>

enum class TileType { FLOOR, WALL, DOOR, TRAP };

struct Tile {
    TileType type;
    Vector2 position;
    Rectangle bounds;

    Tile() : type(TileType::WALL), position({0, 0}), bounds({0, 0, 32, 32}) {}
    Tile(TileType t, Vector2 pos, float size)
        : type(t), position(pos), bounds({pos.x, pos.y, size, size}) {}
};

struct Room {
    int x, y, width, height;
};

class MapGenerator {
private:
    int mapWidth;
    int mapHeight;
    int tileSize;
    std::vector<std::vector<Tile>> tiles;
    std::vector<Room> rooms;
    std::mt19937 rng;
    std::vector<Vector2> decorativeElements;
    std::vector<int> decorativeTypes; // 0=water, 1=magic stone, 2=torch, 3=rune

    void carvePath(int x1, int y1, int x2, int y2);
    void carveRoom(int x, int y, int w, int h);
    void connectRooms();

public:
    MapGenerator(int width, int height, int tSize);

    void generateFloor(int floorNumber);
    void draw();
    void drawDecorations();

    bool isWall(float x, float y) const;
    Vector2 getRandomSpawnPosition();
    std::vector<Vector2> getSpawnPositions(int count);

    Vector2 resolveCollision(Rectangle bounds, Vector2 movement);

    // Getters
    int getMapWidth() const { return mapWidth; }
    int getMapHeight() const { return mapHeight; }
    int getTileSize() const { return tileSize; }
};
