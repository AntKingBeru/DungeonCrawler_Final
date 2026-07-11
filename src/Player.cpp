#include "Player.h"
#include "ConfigUtil.h"

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