#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "ConfigData.h"
#include "Inventory.h"

class Player : public Entity
{
public:
    void loadFrom(const ConfigData& data);

    bool pickUp(const Item& it);
    bool equip(int storageIndex);
    bool unequip(ItemSlot slot, int sub);

    const Inventory& inventory() const
    {
        return inv_;
    }

private:
    void recomputeStats();

	Inventory inv_;
	int baseAtk_ = 5, baseDef_ = 0, baseMaxHp_ = 20;
};

#endif