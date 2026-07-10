#include "Player.h"
#include "ConfigUtil.h"

void Player::loadFrom(const ConfigData& data)
{
    x_ = cfg::requireInt(data, "player", "x");
    y_ = cfg::requireInt(data, "player", "y");
    snapVisual();
    maxHp_ = hp_ = cfg::intOr(data, "player", "hp", 20);
    attack_ = cfg::intOr(data, "player", "attack", 5);
}