#pragma once
#include "raylib.h"
#include <vector>
#include <random>

enum class TileType { FLOOR, WALL, DOOR, TRAP };

// Each floor maps to one of these visual + narrative themes
enum class FloorTheme {
    STONE_DUNGEON,    // Floor 1 (lvl 1–10)  — grey-brown, basic enemies
    CATACOMBS,        // Floor 2 (lvl 11–20) — purple-grey, undead
    SHADOW_PALACE,    // Floor 3 (lvl 21–30) — near-black violet, shadow army + boss
    FROZEN_WASTES,    // Floor 4 (lvl 31–40) — icy blue, frozen monsters
};

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
    int currentFloorNumber;
    FloorTheme currentTheme;
    std::vector<std::vector<Tile>> tiles;
    std::vector<Room> rooms;
    std::mt19937 rng;
    std::vector<Vector2> decorativeElements;
    std::vector<int> decorativeTypes;
    Vector2 throneRoomCenter; // Floor 3 only — boss spawn point

    void carvePath(int x1, int y1, int x2, int y2);
    void carveRoom(int x, int y, int w, int h);
    void connectRooms();
    void setTheme(int floorNum);   // maps floor number → FloorTheme
    void generateBossRoom();       // called on SHADOW_PALACE floors

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
    FloorTheme getTheme() const { return currentTheme; }
    Vector2 getThroneRoomCenter() const { return throneRoomCenter; }
    const std::vector<Room>& getRooms() const { return rooms; }
};
