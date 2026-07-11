#ifndef LOOT_TABLE_H
#define LOOT_TABLE_H

#include <vector>
#include <random>
#include "Item.h"

class LootTable
{
public:
    void add(const Item& item, double chance);

    std::vector<Item> roll(std::mt19937& rng) const;

    bool empty() const
    {
        return entries_.empty();
    }

private:
    struct Entry
    {
        Item item;
        double chance;
    };
    std::vector<Entry> entries_;
};

#endif