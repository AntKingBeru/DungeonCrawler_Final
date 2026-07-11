#include "Item.h"

const char* slotName(ItemSlot s)
{
    switch (s)
    {
        case ItemSlot::Weapon:
            return "weapon";
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