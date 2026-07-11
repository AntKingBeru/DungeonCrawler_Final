#include "Inventory.h"

int Inventory::firstEmpty() const
{
    for (int i = 0; i < SLOTS; ++i)
        if (!storage_[i])
            return i;
    return -1;
}

bool Inventory::addItem(const Item& it)
{
    const int i = firstEmpty();
    if (i < 0)
        return false;
    storage_[i] = it;
    return true;
}

bool Inventory::equip(int i)
{
    if (i < 0 || i >= SLOTS || !storage_[i])
        return false;
    if (storage_[i]->isPotion())
        return false;
    const ItemSlot slot = storage_[i]->slot;
    auto& worn = equipped_[slot];
    if (static_cast<int>(worn.size()) < slotCapacity(slot))
    {
        worn.push_back(*storage_[i]);
        storage_[i].reset();
    }
    else
    {
        Item tmp = worn[0];
        worn[0] = *storage_[i];
        storage_[i] = tmp;
    }
    return true;
}

bool Inventory::unequip(ItemSlot slot, int sub)
{
    auto it = equipped_.find(slot);
    if (it == equipped_.end() || sub < 0 || sub >= static_cast<int>(it->second.size()))
        return false;
    const int dst = firstEmpty();
    if (dst < 0)
        return false;
    storage_[dst] = it->second[sub];
    it->second.erase(it->second.begin() + sub);
    return true;
}

bool Inventory::removeStorage(int i)
{
    if (i < 0 || i >= SLOTS || !storage_[i])
        return false;
    storage_[i].reset();
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
    int t = 0; for (auto& [s, v] : equipped_)
        for (auto& it : v)
            t += it.def;
    return t;
}
int Inventory::totalHp()  const
{
    int t = 0; for (auto& [s, v] : equipped_)
        for (auto& it : v)
            t += it.hp; 
    return t;
}