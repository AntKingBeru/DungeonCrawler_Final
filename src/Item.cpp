#include "Item.h"
#include <sstream>

const char* slotName(ItemSlot s)
{
    switch (s)
    {
        case ItemSlot::Weapon:
            return "weapon";
        case ItemSlot::Shield:
			return "shield";
        case ItemSlot::Helmet:
            return "helmet";
        case ItemSlot::Cape:
            return "cape";
        case ItemSlot::Armor:
            return "armor";
        case ItemSlot::Gloves:
            return "gloves";
        case ItemSlot::Boots:
            return "boots";
        case ItemSlot::Amulet:
            return "amulet";
        case ItemSlot::Ring:
            return "ring";
    }
    return "?";
}

int slotCapacity(ItemSlot s)
{
    return s == ItemSlot::Ring ? 8 : 1;
}

bool parseSlot(const std::string& s, ItemSlot& out)
{
    if (s == "weapon")
    {
        out = ItemSlot::Weapon;
        return true;
    }
    if (s == "shield")
    {
        out = ItemSlot::Shield;
        return true;
    }
    if (s == "helmet")
    {
        out = ItemSlot::Helmet;
        return true;
    }
    if (s == "cape")
    {
        out = ItemSlot::Cape;
        return true;
    }
    if (s == "armor")
    {
        out = ItemSlot::Armor;
        return true;
    }
    if (s == "gloves")
    {
        out = ItemSlot::Gloves;
        return true;
    }
    if (s == "boots")
    {
        out = ItemSlot::Boots;
        return true;
    }
    if (s == "amulet")
    {
        out = ItemSlot::Amulet;
        return true;
    }
    if (s == "ring")
    {
        out = ItemSlot::Ring;
        return true;
    }
    return false;
}

namespace
{
    std::string spacesToUnderscores(std::string s)
    {
        for (char& c : s)
            if (c == ' ')
                c = '_';
        return s;
    }
    std::string underscoresToSpaces(std::string s)
    {
        for (char& c : s)
            if (c == '_')
                c = ' ';
        return s;
    }
}

bool parseItemBody(std::istringstream& ss, Item& out)
{
    std::string first; if (!(ss >> first))
        return false;
    if (first == "potion")
    {
        std::string name; int heal;
        if (!(ss >> name >> heal))
            return false;
        out = Item{ underscoresToSpaces(name), ItemSlot::Weapon, 0, 0, 0, heal, 0 };
        return true;
    }
    if (first == "gold")
    {
        int amount; if (!(ss >> amount))
            return false;
        out = Item{ "gold", ItemSlot::Weapon, 0, 0, 0, 0, amount };
        return true;
    }
    ItemSlot slot;
    if (!parseSlot(first, slot))
        return false;
    std::string name; int atk, def, hp;
    if (!(ss >> name >> atk >> def >> hp))
        return false;
    out = Item{ underscoresToSpaces(name), slot, atk, def, hp, 0, 0 };
    return true;
}

std::string itemToBody(const Item& it)
{
    std::ostringstream os;
    if (it.isPotion())
        os << "potion " << spacesToUnderscores(it.name) << ' ' << it.heal;
    else if (it.isGold())
        os << "gold " << it.gold;
    else
        os << slotName(it.slot) << ' ' << spacesToUnderscores(it.name)
        << ' ' << it.atk << ' ' << it.def << ' ' << it.hp;
    return os.str();
}