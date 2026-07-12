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
    bool usePotion(int storageIndex);
    bool removeStorage(int storageIndex);

    const Inventory& inventory() const
    {
        return inv_;
    }

    int gold() const
    {
        return gold_;
    }
    void addGold(int g)
    {
        gold_ += g;
    }
    bool spendGold(int g);

    int baseMaxHp() const
    {
		return baseMaxHp_;
    }
    int baseAtk() const
    {
		return baseAtk_;
    }
    int baseDef() const
    {
		return baseDef_;
    }

    void writeState(ConfigData& out) const;
	void readState(const ConfigData& in);

    void moveToStart(const ConfigData& data);

private:
    void recomputeStats();

	Inventory inv_;
	int baseAtk_ = 5, baseDef_ = 0, baseMaxHp_ = 20;
    int gold_ = 0;
};

#endif