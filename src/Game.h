#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include <map>
#include <random>
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Item.h"
#include "LootTable.h"

struct GroundItem
{
    Item item;
    int x;
    int y;
};

class Game
{
public:
    void load(const std::string& configPath);
    void update(float dt);

    void movePlayer(int dx, int dy);
    void attackAt(int tileX, int tileY);

    void equipItem(int storageIndex)
    {
        player_.equip(storageIndex);
    }
    void unequipItem(ItemSlot slot, int sub)
    {
        player_.unequip(slot, sub);
    }

    bool isComplete() const
    {
        return complete_;
    }
    bool isGameOver() const
    {
        return gameOver_;
    }
    const Map& map() const
    {
        return map_;
    }
    const Player& player() const
    {
        return player_;
    }
    const std::vector<Enemy>& enemies() const
    {
        return enemies_;
    }
    const std::vector<GroundItem>& groundItems() const
    {
        return ground_;
    }
    const std::vector<std::string>& log() const
    {
        return log_;
    }

private:
    void enemyTurn();
    void removeDead();
    void addLog(const std::string& msg);
    void tryPickUp(int x, int y);
    void dropLoot(const Enemy& e);
    bool wallOrEnemy(int x, int y, const Enemy* self) const;
    const Enemy* enemyAt(int x, int y) const;
    Enemy* enemyAt(int x, int y);

    Map map_;
    Player player_;
    std::vector<Enemy> enemies_;
    std::vector<GroundItem> ground_;
    std::map<std::string, LootTable> lootTables_;
    std::vector<std::string> log_;
    std::mt19937 rng_{ std::random_device{}() };
    bool complete_ = false;
    bool gameOver_ = false;
};

#endif