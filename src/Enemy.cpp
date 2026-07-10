#include "Enemy.h"
#include <utility>

Enemy::Enemy(std::string type, int x, int y, int hp, int attack, int sight, std::string reward)
    : type_(std::move(type)), reward_(std::move(reward)), sight_(sight)
{
    x_ = x; y_ = y;
    snapVisual();
    maxHp_ = hp_ = hp;
    attack_ = attack;
}

Enemy Enemy::makeGoblin(int x, int y)
{
    return Enemy("goblin", x, y, 10, 3, 6, "3 gold");
}