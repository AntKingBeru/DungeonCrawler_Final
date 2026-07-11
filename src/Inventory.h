#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <map>
#include "Item.h"

class Inventory
{
public:
    static constexpr int STORAGE_MAX = 27;

    bool addItem(const Item& it);

    bool equip(int storageIndex);

    bool unequip(ItemSlot slot, int sub);

    const std::vector<Item>& storage() const
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
    std::vector<Item> storage_;
    std::map<ItemSlot, std::vector<Item>> equipped_;
};

#endif