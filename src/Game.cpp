#include "Game.h"
#include "ConfigUtil.h"
#include "ParserFactory.h"
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <algorithm>

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
    std::string prettyName(std::string s)
    {
        for (char& c : s)
            if (c == '_') c = ' ';
        return s;
    }
}

void Game::load(const std::string& configPath)
{
    auto parser = makeParser(configPath);
    ConfigData data = parser->parse(configPath);

    map_.loadFrom(data);
    player_.loadFrom(data);
    enemies_.clear();
    ground_.clear();
    lootTables_.clear();
    log_.clear();
    complete_ = gameOver_ = false;

    if (auto it = data.find("enemies"); it != data.end())
        for (const auto& [id, spec] : it->second)
        {
            std::istringstream ss(spec);
            std::string type; int ex, ey;
            if (!(ss >> type >> ex >> ey))
                continue;
            if (type == "goblin")
                enemies_.push_back(Enemy::makeGoblin(ex, ey));
            else if (type == "skeleton")
                enemies_.push_back(Enemy::makeSkeleton(ex, ey));
            else if (type == "dragon")
                enemies_.push_back(Enemy::makeDragon(ex, ey));
        }

    if (auto it = data.find("items"); it != data.end())
        for (const auto& [id, spec] : it->second)
        {
            std::istringstream ss(spec);
            std::string slotStr, name; int ix, iy, atk, def, hp;
            if (!(ss >> slotStr >> ix >> iy >> name >> atk >> def >> hp))
                continue;
            ItemSlot slot;
            if (parseSlot(slotStr, slot))
                ground_.push_back({ Item{ prettyName(name), slot, atk, def, hp }, ix, iy });
        }

    for (const auto& [section, kvs] : data)
    {
        const std::string prefix = "loot_";
        if (section.rfind(prefix, 0) != 0)
            continue;
        const std::string type = section.substr(prefix.size());
        LootTable table;
        for (const auto& [id, spec] : kvs)
        {
            std::istringstream ss(spec);
            std::string slotStr, name; int atk, def, hp; double chance;
            if (!(ss >> slotStr >> name >> atk >> def >> hp >> chance))
                continue;
            ItemSlot slot;
            if (parseSlot(slotStr, slot))
                table.add(Item{ prettyName(name), slot, atk, def, hp }, chance);
        }
        lootTables_[type] = table;
    }

    addLog("Entered " + cfg::require(data, "meta", "name") + ".");
}

void Game::update(float dt)
{
    player_.update(dt);
    for (auto& e : enemies_)
        e.update(dt);
}

void Game::movePlayer(int dx, int dy)
{
    if (complete_ || gameOver_ || player_.isMoving())
        return;
    const int nx = player_.x() + dx, ny = player_.y() + dy;
    if (map_.isWall(nx, ny))
        return;
    if (enemyAt(nx, ny))
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

void Game::attackAt(int tileX, int tileY)
{
    if (complete_ || gameOver_ || player_.isMoving())
        return;
    Enemy* e = enemyAt(tileX, tileY);
    if (!e)
        return;
    if (std::abs(e->x() - player_.x()) + std::abs(e->y() - player_.y()) != 1)
        return;

    const int dmg = std::max(1, player_.attackPower() - e->defense());
    e->takeDamage(dmg);
    addLog("You hit the " + e->type() + " for " + std::to_string(dmg) + ".");
    if (!e->alive())
    {
        addLog(cap(e->type()) + " slain!");
        dropLoot(*e);
    }

    enemyTurn();
    removeDead();
}

void Game::dropLoot(const Enemy& e)
{
    auto it = lootTables_.find(e.type());
    if (it == lootTables_.end() || it->second.empty())
        return;
    std::vector<Item> drops = it->second.roll(rng_);
    if (drops.empty())
    {
        addLog("...it dropped nothing.");
        return;
    }
    for (const auto& item : drops)
    {
        ground_.push_back({ item, e.x(), e.y() });
        addLog("Dropped: " + item.name + ".");
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