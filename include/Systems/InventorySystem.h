#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <unordered_map>

struct InventoryItemData {
    std::string name;
    std::string description;
    int maxStack;
    int rarity; // 1=common, 2=rare, 3=epic
    Color displayColor;
};

struct InventorySlot {
    InventoryItemData itemData;
    int quantity;
};

class InventorySystem {
private:
    std::vector<InventorySlot> slots;
    int maxSlots;
public:
    InventorySystem(int maxSlots = 24);

    bool addItem(const InventoryItemData& item, int quantity = 1);
    bool removeItem(const std::string& itemName, int quantity = 1);
    bool hasItem(const std::string& itemName, int minQty = 1) const;
    int getItemCount(const std::string& itemName) const;
    const std::vector<InventorySlot>& getSlots() const;
    void clear();

    // Optionally: sorting, filtering, expand capacity, etc.
};
