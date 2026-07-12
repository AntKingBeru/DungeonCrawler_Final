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
struct Chest
{
    int x;
    int y;
    std::string table;
};
struct ShopKeeper
{
    int x = 0;
    int y = 0;
    bool exists = false;
};
struct ShopEntry
{
	Item item;
	int price;
};

class Game
{
public:
    void newGame(const std::string& levelPath);
    bool loadSlot(const std::string& savePath);
    void saveSlot(const std::string& savePath) const;
    const std::string& levelName() const
    {
        return levelName_;
    }

    void update(float dt);

    void movePlayer(int dx, int dy);
    void interactAt(int tileX, int tileY);

    void useBackpackItem(int storageIndex);
    void unequipItem(ItemSlot slot, int sub)
    {
        player_.unequip(slot, sub);
    }

    bool isShopOpen() const
    {
        return shopOpen_;
    }
    void closeShop()
    {
        shopOpen_ = false;
    }
    void buy(int stockIndex);
	void sellBackpack(int storageIndex);
    int gold() const
    {
        return player_.gold();
    }
    static int sellValue(const Item& it);

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
    const std::vector<Chest>& chests() const
    {
        return chests_;
    }
    const ShopKeeper& shopkeeper() const
    {
        return shopkeeper_;
    }
    const std::vector<ShopEntry>& shopStock() const
    {
        return shopStock_;
    }
    const std::vector<std::string>& log() const
    {
        return log_;
    }

private:
    void buildStatic(const ConfigData& level);
    void buildEnemies(const ConfigData& data);
    void buildGround(const ConfigData& data, const std::string& section);
    void buildChests(const ConfigData& data, const std::string& section);
    void resetRuntime();

    void enemyTurn();
    void removeDead();
    void addLog(const std::string& msg);
    void tryPickUp(int x, int y);
    void dropLoot(const std::string& table, int x, int y);
    void openChest(size_t index);
    bool blocked(int x, int y) const;
    bool wallOrEnemy(int x, int y, const Enemy* self) const;
    const Enemy* enemyAt(int x, int y) const;
    Enemy* enemyAt(int x, int y);

    Map map_;
    Player player_;
    std::vector<Enemy> enemies_;
    std::vector<GroundItem> ground_;
    std::vector<Chest> chests_;
    ShopKeeper shopkeeper_;
    std::vector<ShopEntry> shopStock_;
    std::map<std::string, LootTable> lootTables_;
    std::vector<std::string> log_;
    std::string levelPath_;
    std::string levelName_;
    std::mt19937 rng_{ std::random_device{}() };
    bool complete_ = false;
    bool gameOver_ = false;
    bool shopOpen_ = false;
};

#endif