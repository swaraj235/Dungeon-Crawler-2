#include "InventorySystem.h"
#include <algorithm>

InventorySystem::InventorySystem(int maxSlots) : maxSlots(maxSlots) {}

bool InventorySystem::addItem(const InventoryItemData& item, int quantity) {
    // Check if item already exists
    for (auto& slot : slots) {
        if (slot.itemData.name == item.name && slot.quantity < item.maxStack) {
            int canAdd = item.maxStack - slot.quantity;
            int toAdd = std::min(canAdd, quantity);
            slot.quantity += toAdd;
            quantity -= toAdd;

            if (quantity <= 0) return true;
        }
    }

    // Add to new slot
    if (slots.size() < maxSlots && quantity > 0) {
        slots.push_back({item, std::min(quantity, item.maxStack)});
        return true;
    }

    return quantity <= 0;
}

bool InventorySystem::removeItem(const std::string& itemName, int quantity) {
    for (auto& slot : slots) {
        if (slot.itemData.name == itemName && slot.quantity > 0) {
            int remove = std::min(slot.quantity, quantity);
            slot.quantity -= remove;
            quantity -= remove;

            if (slot.quantity <= 0) {
                slots.erase(
                    std::remove_if(slots.begin(), slots.end(),
                        [](const InventorySlot& s) { return s.quantity <= 0; }),
                    slots.end()
                );
            }

            if (quantity <= 0) return true;
        }
    }

    return false;
}

bool InventorySystem::hasItem(const std::string& itemName, int minQty) const {
    for (const auto& slot : slots) {
        if (slot.itemData.name == itemName && slot.quantity >= minQty) {
            return true;
        }
    }
    return false;
}

int InventorySystem::getItemCount(const std::string& itemName) const {
    for (const auto& slot : slots) {
        if (slot.itemData.name == itemName) {
            return slot.quantity;
        }
    }
    return 0;
}

const std::vector<InventorySlot>& InventorySystem::getSlots() const {
    return slots;
}

void InventorySystem::clear() {
    slots.clear();
}
