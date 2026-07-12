#include "Player.h"
#include "ConfigUtil.h"
#include <sstream>

void Player::loadFrom(const ConfigData& data)
{
    x_ = cfg::requireInt(data, "player", "x");
    y_ = cfg::requireInt(data, "player", "y");
    snapVisual();
    baseMaxHp_ = cfg::intOr(data, "player", "hp", 20);
    baseAtk_ = cfg::intOr(data, "player", "attack", 5);
    baseDef_ = cfg::intOr(data, "player", "defense", 0);
    gold_ = cfg::intOr(data, "player", "gold", 0);
    recomputeStats();
    hp_ = maxHp_;
}

bool Player::pickUp(const Item& it)
{
    return inv_.addItem(it);
}
bool Player::equip(int i)
{
    bool ok = inv_.equip(i);
    if (ok)
        recomputeStats();
    return ok;
}
bool Player::unequip(ItemSlot slot, int sub)
{
    bool ok = inv_.unequip(slot, sub);
    if (ok)
        recomputeStats();
    return ok;
}
bool Player::usePotion(int i)
{
    const auto& st = inv_.storage();
    if (i < 0 || i >= static_cast<int>(st.size()) || !st[i] || !st[i]->isPotion())
        return false;
    heal(st[i]->heal);
    inv_.removeStorage(i);
    return true;
}
bool Player::removeStorage(int i)
{
	return inv_.removeStorage(i);
}
bool Player::spendGold(int g)
{
	if (g > gold_)
		return false;
	gold_ -= g;
	return true;
}

void Player::writeState(ConfigData& out) const
{
    Section& p = out["player"];
    p["x"] = std::to_string(x_);
    p["y"] = std::to_string(y_);
    p["basehp"] = std::to_string(baseMaxHp_);
    p["curhp"] = std::to_string(hp_);
    p["attack"] = std::to_string(baseAtk_);
    p["defense"] = std::to_string(baseDef_);
    p["gold"] = std::to_string(gold_);

    int n = 0;
    const auto& st = inv_.storage();
    for (int k = 0; k < static_cast<int>(st.size()); ++k)
        if (st[k])
            out["backpack"]["b" + std::to_string(n++)] =
            std::to_string(k) + " " + itemToBody(*st[k]);

    n = 0;
    for (const auto& [slot, worn] : inv_.equipped())
        for (const auto& it : worn)
            out["equipped"]["e" + std::to_string(n++)] = itemToBody(it);
}

void Player::readState(const ConfigData& in)
{
    x_ = cfg::requireInt(in, "player", "x");
    y_ = cfg::requireInt(in, "player", "y");
    snapVisual();
    baseMaxHp_ = cfg::intOr(in, "player", "basehp", 20);
    baseAtk_ = cfg::intOr(in, "player", "attack", 5);
    baseDef_ = cfg::intOr(in, "player", "defense", 0);
    gold_ = cfg::intOr(in, "player", "gold", 0);

    inv_.clear();
    if (auto it = in.find("backpack"); it != in.end())
        for (const auto& [id, spec] : it->second)
        {
            std::istringstream ss(spec);
            int slot;
            if (!(ss >> slot))
                continue;
            Item item;
            if (parseItemBody(ss, item))
                inv_.placeBackpack(slot, item);
        }
    if (auto it = in.find("equipped"); it != in.end())
        for (const auto& [id, spec] : it->second)
        {
            std::istringstream ss(spec);
            Item item;
            if (parseItemBody(ss, item))
                inv_.placeEquipped(item);
        }

    recomputeStats();
    const int cur = cfg::intOr(in, "player", "curhp", maxHp_);
    hp_ = cur;
    if (hp_ > maxHp_)
        hp_ = maxHp_;
    if (hp_ < 0)
        hp_ = 0;
}

void Player::recomputeStats()
{
    attack_ = baseAtk_ + inv_.totalAtk();
    defense_ = baseDef_ + inv_.totalDef();

    const int oldMax = maxHp_;
    const int newMax = baseMaxHp_ + inv_.totalHp();
    maxHp_ = newMax;
    if (newMax > oldMax)
        hp_ += (newMax - oldMax);
    if (hp_ > maxHp_)
        hp_ = maxHp_;
}