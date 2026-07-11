#include "LootTable.h"

void LootTable::add(const Item& item, double chance)
{
    entries_.push_back({ item, chance });
}

std::vector<Item> LootTable::roll(std::mt19937& rng) const
{
    std::vector<Item> out;
    std::uniform_real_distribution<double> d(0.0, 1.0);
    for (const auto& e : entries_)
        if (d(rng) < e.chance)
            out.push_back(e.item);
    return out;
}