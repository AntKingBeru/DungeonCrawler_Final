#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <sstream>

enum class ItemSlot
{
	Weapon,
	Shield,
	Helmet,
	Cape,
	Armor,
	Gloves,
	Boots,
	Amulet,
	Ring
};

enum class KeyType
{
	None,
	Door,
	Chest
};

struct Item
{
	std::string name;
	ItemSlot slot = ItemSlot::Weapon;
	int atk = 0, def = 0, hp = 0;
	int heal = 0;
	int gold = 0;
	KeyType key = KeyType::None;

	bool isPotion() const
	{
		return heal > 0;
	}
	bool isGold() const
	{
		return gold > 0;
	}
	bool isKey() const
	{
		return key != KeyType::None;
	}
	bool isDoorKey() const
	{
		return key == KeyType::Door;
	}
	bool isChestKey() const
	{
		return key == KeyType::Chest;
	}
};

const char* slotName(ItemSlot s);
int slotCapacity(ItemSlot s);
bool parseSlot(const std::string& s, ItemSlot& out);

bool parseItemBody(std::istringstream& ss, Item& out);
std::string itemToBody(const Item& it);

#endif