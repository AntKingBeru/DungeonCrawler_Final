#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <map>
#include <optional>
#include "Item.h"

class Inventory
{
public:
    static constexpr int SLOTS = 27;
    Inventory() : storage_(SLOTS) {}

    bool addItem(const Item& it);
    bool equip(int slot);
    bool unequip(ItemSlot slot, int sub);
    bool removeStorage(int slot);

    void clear();
    bool placeBackpack(int slot, const Item& it);
	bool placeEquipped(const Item& it);

    const std::vector<std::optional<Item>>& storage() const
    {
        return storage_;
    }
    const std::map<ItemSlot, std::vector<Item>>& equipped() const
    {
        return equipped_;
    }
    int  equippedCount(ItemSlot s) const;

    int totalAtk() const;
    int totalDef() const;
    int totalHp() const;

private:
    int firstEmpty() const;
    std::vector<std::optional<Item>> storage_;
    std::map<ItemSlot, std::vector<Item>> equipped_;
};

#endif