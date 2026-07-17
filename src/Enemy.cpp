#include "Enemy.h"
#include <utility>
#include <cmath>
#include <algorithm>

Enemy::Enemy(std::string type, int x, int y, int hp, int attack, int sight, int defense, int roamRange)
    : type_(std::move(type)), sight_(sight),
    spawnX_(x), spawnY_(y), roamRange_(roamRange)
{
    x_ = x; y_ = y; snapVisual();
    maxHp_ = hp_ = hp;
    attack_ = attack;
    defense_ = defense;
}

Enemy Enemy::makeGoblin(int x, int y)
{
    return Enemy("goblin", x, y, 10, 3, 6, 0, 5);
}
Enemy Enemy::makeSkeleton(int x, int y)
{
    return Enemy("skeleton", x, y, 14, 4, 7, 1, 3);
}
Enemy Enemy::makeDragon(int x, int y)
{
    return Enemy("dragon", x, y, 40, 7, 10, 3, 0);
}

void Enemy::scaleStats(float m)
{
    maxHp_ = std::max(1, static_cast<int>(std::lround(maxHp_ * m)));
    hp_ = maxHp_;
    attack_ = std::max(1, static_cast<int>(std::lround(attack_ * m)));
}