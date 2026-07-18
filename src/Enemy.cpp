#include "Enemy.h"
#include <utility>
#include <cmath>
#include <algorithm>

Enemy::Enemy(std::string type, std::string name, int x, int y,
    int hp, int attack, int sight, int defense, int roamRange)
    : type_(std::move(type)), name_(std::move(name)), sight_(sight),
    spawnX_(x), spawnY_(y), roamRange_(roamRange)
{
    x_ = x; y_ = y;
    snapVisual();
    maxHp_ = hp_ = hp;
    attack_ = attack;
    defense_ = defense;
}

std::optional<Enemy> Enemy::create(const std::string& type, int x, int y)
{
    if (type == "goblin")
        return Enemy("goblin", "Shadow Goblin", x, y, 10, 3, 6, 0, 5);
    if (type == "hobgoblin")
        return Enemy("hobgoblin", "Shadow Hobgoblin", x, y, 24, 6, 7, 2, 4);
    if (type == "golem")
        return Enemy("golem", "Shadow Golem", x, y, 65, 6, 9, 5, 0);
    if (type == "knight")
        return Enemy("knight", "Shadow Knight", x, y, 16, 5, 7, 2, 3);
    if (type == "archer")
    {
        Enemy e("archer", "Shadow Archer", x, y, 12, 5, 8, 0, 3);
        e.attackRange_ = 3;
        e.missChance_ = 0.3;
        return e;
    }
    if (type == "wyvern")
    {
        Enemy e("wyvern", "Shadow Wyvern", x, y, 48, 8, 10, 3, 0);
        e.moveSpeed_ = 2;
        return e;
    }
    if (type == "mage")
    {
        Enemy e("mage", "Shadow Mage", x, y, 16, 6, 8, 1, 4);
        e.attackRange_ = 2;
        e.missChance_ = 0.25;
        e.teleportChance_ = 0.35;
        e.teleportRange_ = 4;
        return e;
    }
    if (type == "lich")
    {
        Enemy e("lich", "Shadow Lich", x, y, 60, 9, 10, 4, 0);
        e.attackRange_ = 2;
        e.missChance_ = 0.2;
        e.teleportChance_ = 0.3;
        e.teleportRange_ = 3;
        return e;
    }
    return std::nullopt;
}

void Enemy::scaleStats(float m)
{
    maxHp_ = std::max(1, static_cast<int>(std::lround(maxHp_ * m)));
    hp_ = maxHp_;
    attack_ = std::max(1, static_cast<int>(std::lround(attack_ * m)));
}