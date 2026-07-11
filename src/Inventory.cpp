#include "Inventory.h"

bool Inventory::addItem(const Item& it)
{
    if (static_cast<int>(storage_.size()) >= STORAGE_MAX)
        return false;
    storage_.push_back(it);
    return true;
}

bool Inventory::equip(int i)
{
    if (i < 0 || i >= static_cast<int>(storage_.size()))
        return false;
    const ItemSlot slot = storage_[i].slot;
    auto& worn = equipped_[slot];
    if (static_cast<int>(worn.size()) < slotCapacity(slot))
    {
        worn.push_back(storage_[i]);
        storage_.erase(storage_.begin() + i);
    }
    else
    {
        std::swap(storage_[i], worn[0]);
    }
    return true;
}

bool Inventory::unequip(ItemSlot slot, int sub)
{
    auto it = equipped_.find(slot);
    if (it == equipped_.end() || sub < 0 || sub >= static_cast<int>(it->second.size()))
        return false;
    if (static_cast<int>(storage_.size()) >= STORAGE_MAX)
        return false;
    storage_.push_back(it->second[sub]);
    it->second.erase(it->second.begin() + sub);
    return true;
}

int Inventory::equippedCount(ItemSlot s) const
{
    auto it = equipped_.find(s);
    return it == equipped_.end() ? 0 : static_cast<int>(it->second.size());
}

int Inventory::totalAtk() const
{
    int t = 0;
    for (auto& [s, v] : equipped_)
        for (auto& it : v)
            t += it.atk;
    return t;
}
int Inventory::totalDef() const
{
    int t = 0;
    for (auto& [s, v] : equipped_)
        for (auto& it : v)
            t += it.def;
    return t;
}
int Inventory::totalHp() const
{
    int t = 0;
    for (auto& [s, v] : equipped_)
        for (auto& it : v)
            t += it.hp;
    return t;
}