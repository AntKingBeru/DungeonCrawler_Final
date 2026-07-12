#include "Game.h"
#include "ConfigUtil.h"
#include "ParserFactory.h"
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <fstream>

namespace
{
    int sgn(int v)
    {
        return (v > 0) - (v < 0);
    }
    std::string cap(std::string s)
    {
        if (!s.empty()) s[0] = static_cast<char>(std::toupper(s[0]));
        return s;
    }
}

int Game::sellValue(const Item& it)
{
    const int base = it.atk * 3 + it.def * 3 + it.hp + it.heal;
    return std::max(1, base / 2);
}

void Game::buildStatic(const ConfigData& data)
{
    map_.loadFrom(data);
    lootTables_.clear();
    shopStock_.clear();
    shopkeeper_ = {};

    if (auto it = data.find("shopkeeper");
        it != data.end())
        shopkeeper_ = { cfg::requireInt(data, "shopkeeper", "x"),
                        cfg::requireInt(data, "shopkeeper", "y"), true };

    if (auto it = data.find("shop");
        it != data.end())
        for (const auto& [id, spec] : it->second)
        {
            std::istringstream ss(spec);
            int price;
            if (!(ss >> price))
                continue;
            Item item;
            if (parseItemBody(ss, item) && !item.isGold())
                shopStock_.push_back({ item, price });
        }

    for (const auto& [section, kvs] : data)
    {
        const std::string prefix = "loot_";
        if (section.rfind(prefix, 0) != 0)
            continue;
        const std::string name = section.substr(prefix.size());
        LootTable table;
        for (const auto& [id, spec] : kvs)
        {
            std::istringstream ss(spec);
            Item item;
            if (!parseItemBody(ss, item))
                continue;
            double chance;
            if (!(ss >> chance))
                continue;
            table.add(item, chance);
        }
        lootTables_[name] = table;
    }
}

void Game::buildEnemies(const ConfigData& data)
{
    enemies_.clear();
    auto it = data.find("enemies");
    if (it == data.end())
        return;
    for (const auto& [id, spec] : it->second)
    {
        std::istringstream ss(spec);
        std::string type;
        int x, y;
        if (!(ss >> type >> x >> y))
            continue;
        Enemy* e = nullptr;
        if (type == "goblin")
        {
            enemies_.push_back(Enemy::makeGoblin(x, y));
            e = &enemies_.back();
        }
        else if (type == "skeleton")
        {
            enemies_.push_back(Enemy::makeSkeleton(x, y));
            e = &enemies_.back();
        }
        else if (type == "dragon")
        {
            enemies_.push_back(Enemy::makeDragon(x, y));
            e = &enemies_.back();
        }
        if (!e)
            continue;
        int hp;
        if (ss >> hp)
            e->setHp(hp);
    }
}

void Game::buildGround(const ConfigData& data, const std::string& section)
{
    ground_.clear();
    auto it = data.find(section);
    if (it == data.end())
        return;
    for (const auto& [id, spec] : it->second)
    {
        std::istringstream ss(spec);
        int x, y;
        if (!(ss >> x >> y))
            continue;
        Item item;
        if (parseItemBody(ss, item) && !item.isGold())
            ground_.push_back({ item, x, y });
    }
}

void Game::buildChests(const ConfigData& data, const std::string& section)
{
    chests_.clear();
    auto it = data.find(section);
    if (it == data.end())
        return;
    for (const auto& [id, spec] : it->second)
    {
        std::istringstream ss(spec);
        int x, y; std::string table;
        if (ss >> x >> y >> table)
            chests_.push_back({ x, y, table });
    }
}

void Game::resetRuntime()
{
    log_.clear();
    complete_ = gameOver_ = shopOpen_ = false;
}

void Game::newGame(const std::string& levelPath)
{
    levelPath_ = levelPath;
    ConfigData data = makeParser(levelPath)->parse(levelPath);
    levelName_ = cfg::require(data, "meta", "name");

    buildStatic(data);
    player_.loadFrom(data);
    buildEnemies(data);
    buildGround(data, "items");
    buildChests(data, "chests");
    resetRuntime();
    addLog("Entered " + levelName_ + ".");
}

bool Game::loadSlot(const std::string& savePath)
{
    std::ifstream probe(savePath);
    if (!probe.good())
        return false;
    probe.close();

    ConfigData save;
    try
    {
        save = makeParser(savePath)->parse(savePath);
    }
    catch (...)
    {
        return false;
    }

    levelPath_ = cfg::require(save, "save", "level");
    ConfigData level = makeParser(levelPath_)->parse(levelPath_);
    levelName_ = cfg::require(level, "meta", "name");

    buildStatic(level);
    player_.readState(save);
    buildEnemies(save);
    buildGround(save, "ground");
    buildChests(save, "chests");
    resetRuntime();
    addLog("Loaded " + levelName_ + ".");
    return true;
}

void Game::saveSlot(const std::string& savePath) const
{
    ConfigData d;
    d["save"]["level"] = levelPath_;
    d["save"]["name"] = levelName_;
    d["save"]["version"] = "1";

    player_.writeState(d);

    int n = 0;
    for (const auto& e : enemies_)
        if (e.alive())
            d["enemies"]["e" + std::to_string(n++)] =
            e.type() + " " + std::to_string(e.x()) + " " + std::to_string(e.y())
            + " " + std::to_string(e.hp());

    n = 0;
    for (const auto& g : ground_)
        d["ground"]["g" + std::to_string(n++)] =
        std::to_string(g.x) + " " + std::to_string(g.y) + " " + itemToBody(g.item);

    n = 0;
    for (const auto& c : chests_)
        d["chests"]["c" + std::to_string(n++)] =
        std::to_string(c.x) + " " + std::to_string(c.y) + " " + c.table;

    makeParser(savePath)->serialize(d, savePath);
}

void Game::update(float dt)
{
    player_.update(dt);
    for (auto& e : enemies_)
        e.update(dt);
}

bool Game::blocked(int x, int y) const
{
    if (map_.isWall(x, y))
        return true;
    if (enemyAt(x, y))
        return true;
    for (const auto& c : chests_)
        if (c.x == x && c.y == y)
            return true;
    if (shopkeeper_.exists && shopkeeper_.x == x && shopkeeper_.y == y)
        return true;
    return false;
}

void Game::movePlayer(int dx, int dy)
{
    if (complete_ || gameOver_ || player_.isMoving())
        return;
    const int nx = player_.x() + dx, ny = player_.y() + dy;
    if (blocked(nx, ny))
        return;

    player_.setTile(nx, ny);
    tryPickUp(nx, ny);
    if (map_.isExit(nx, ny))
    {
        complete_ = true;
        addLog("You reached the exit!");
        return;
    }
    enemyTurn();
}

void Game::interactAt(int tileX, int tileY)
{
    if (complete_ || gameOver_ || player_.isMoving())
        return;
    if (std::abs(tileX - player_.x()) + std::abs(tileY - player_.y()) != 1)
        return;

    if (Enemy* e = enemyAt(tileX, tileY))
    {
        const int dmg = std::max(1, player_.attackPower() - e->defense());
        e->takeDamage(dmg);
        addLog("You hit the " + e->type() + " for " + std::to_string(dmg) + ".");
        if (!e->alive())
        {
            addLog(cap(e->type()) + " slain!");
            dropLoot(e->type(), e->x(), e->y());
        }
        enemyTurn();
        removeDead();
        return;
    }
    if (shopkeeper_.exists && shopkeeper_.x == tileX && shopkeeper_.y == tileY)
    {
        shopOpen_ = true;
        return;
    }
    for (size_t i = 0; i < chests_.size(); ++i)
        if (chests_[i].x == tileX && chests_[i].y == tileY)
        {
            openChest(i);
            return;
        }
}

void Game::openChest(size_t index)
{
    const Chest c = chests_[index];
    chests_.erase(chests_.begin() + index);
    addLog("Opened a chest.");
    dropLoot(c.table, c.x, c.y);
    enemyTurn();
}

void Game::dropLoot(const std::string& table, int x, int y)
{
    auto it = lootTables_.find(table);
    if (it == lootTables_.end() || it->second.empty())
        return;
    std::vector<Item> drops = it->second.roll(rng_);
    if (drops.empty())
    {
        addLog("...nothing.");
        return;
    }
    for (const auto& item : drops)
    {
        if (item.isGold())
        {
            player_.addGold(item.gold);
            addLog("Found " + std::to_string(item.gold) + " gold.");
        }
        else
        {
            ground_.push_back({ item, x, y });
            addLog("Dropped: " + item.name + ".");
        }
    }
}

void Game::buy(int i)
{
    if (i < 0 || i >= static_cast<int>(shopStock_.size()))
        return;
    const ShopEntry e = shopStock_[i];
    if (player_.gold() < e.price)
    {
        addLog("Not enough gold.");
        return;
    }
    if (!player_.pickUp(e.item))
    {
        addLog("Backpack full.");
        return;
    }
    player_.spendGold(e.price);
    addLog("Bought " + e.item.name + " for " + std::to_string(e.price) + " gold.");
}

void Game::sellBackpack(int i)
{
    const auto& st = player_.inventory().storage();
    if (i < 0 || i >= static_cast<int>(st.size()) || !st[i])
        return;
    const int v = sellValue(*st[i]);
    const std::string name = st[i]->name;
    player_.removeStorage(i);
    player_.addGold(v);
    addLog("Sold " + name + " for " + std::to_string(v) + " gold.");
}

void Game::useBackpackItem(int i)
{
    const auto& st = player_.inventory().storage();
    if (i < 0 || i >= static_cast<int>(st.size()) || !st[i])
        return;
    if (st[i]->isPotion())
    {
        if (player_.hp() >= player_.maxHp())
        {
            addLog("Already at full health.");
            return;
        }
        const std::string name = st[i]->name;
        const int amount = st[i]->heal;
        player_.usePotion(i);
        addLog("Drank " + name + " (+" + std::to_string(amount) + " HP).");
    }
    else
    {
        player_.equip(i);
    }
}

void Game::tryPickUp(int x, int y)
{
    for (size_t i = 0; i < ground_.size();)
    {
        if (ground_[i].x == x && ground_[i].y == y)
        {
            if (player_.pickUp(ground_[i].item))
            {
                addLog("Picked up " + ground_[i].item.name + ".");
                ground_.erase(ground_.begin() + i);
                continue;
            }
            addLog("Backpack full - left " + ground_[i].item.name + ".");
        }
        ++i;
    }
}

void Game::enemyTurn()
{
    for (auto& e : enemies_)
    {
        if (!e.alive())
            continue;
        const int dx = player_.x() - e.x(), dy = player_.y() - e.y();
        const int manhattan = std::abs(dx) + std::abs(dy);

        if (manhattan == 1)
        {
            const int dmg = std::max(1, e.attackPower() - player_.defense());
            player_.takeDamage(dmg);
            addLog(cap(e.type()) + " hits you for " + std::to_string(dmg) + ".");
            if (!player_.alive())
            {
                gameOver_ = true;
                addLog("You died.");
                return;
            }
            continue;
        }

        const int chebyshev = std::max(std::abs(dx), std::abs(dy));
        if (chebyshev > e.sight())
            continue;

        const int sx = sgn(dx), sy = sgn(dy);
        auto tryStep = [&](int mx, int my)
            {
                const int tx = e.x() + mx, ty = e.y() + my;
                if (tx == player_.x() && ty == player_.y())
                    return false;
                if (wallOrEnemy(tx, ty, &e))
                    return false;
                e.setTile(tx, ty);
                return true;
            };
        if (std::abs(dx) >= std::abs(dy))
        {
            if (!tryStep(sx, 0))
                tryStep(0, sy);
        }
        else
        {
            if (!tryStep(0, sy))
                tryStep(sx, 0);
        }
    }
}

void Game::removeDead()
{
    for (size_t i = 0; i < enemies_.size();)
        if (!enemies_[i].alive())
            enemies_.erase(enemies_.begin() + i);
        else
            ++i;
}

void Game::addLog(const std::string& msg)
{
    log_.push_back(msg);
    if (log_.size() > 50)
        log_.erase(log_.begin());
}

bool Game::wallOrEnemy(int x, int y, const Enemy* self) const
{
    if (map_.isWall(x, y))
        return true;
    for (const auto& e : enemies_)
        if (&e != self && e.alive() && e.x() == x && e.y() == y)
            return true;
    for (const auto& c : chests_) if (c.x == x && c.y == y)
        return true;
    if (shopkeeper_.exists && shopkeeper_.x == x && shopkeeper_.y == y)
        return true;
    return false;
}

const Enemy* Game::enemyAt(int x, int y) const
{
    for (const auto& e : enemies_)
        if (e.alive() && e.x() == x && e.y() == y)
            return &e;
    return nullptr;
}
Enemy* Game::enemyAt(int x, int y)
{
    for (auto& e : enemies_)
        if (e.alive() && e.x() == x && e.y() == y)
            return &e;
    return nullptr;
}