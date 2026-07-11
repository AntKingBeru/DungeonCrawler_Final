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

void Player::recomputeStats()
{
    attack_ = baseAtk_ + inv_.totalAtk();
    defense_ = baseDef_ + inv_.totalDef();
    maxHp_ = baseMaxHp_ + inv_.totalHp();
    if (hp_ > maxHp_)
        hp_ = maxHp_;
}