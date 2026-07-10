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
        if (!s.empty())
            s[0] = static_cast<char>(std::toupper(s[0]));
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
    log_.clear();
    complete_ = gameOver_ = false;

    auto it = data.find("enemies");
    if (it != data.end())
    {
        for (const auto& [id, spec] : it->second)
        {
            std::istringstream ss(spec);
            std::string type; int ex, ey;
            if (!(ss >> type >> ex >> ey))
                continue;
            if (type == "goblin") enemies_.push_back(Enemy::makeGoblin(ex, ey));
        }
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

    e->takeDamage(player_.attackPower());
    addLog("You hit the " + e->type() + " for " + std::to_string(player_.attackPower()) + ".");
    if (!e->alive())
        addLog(cap(e->type()) + " slain! Dropped: " + e->reward() + ".");

    enemyTurn();
    removeDead();
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
            player_.takeDamage(e.attackPower());
            addLog(cap(e.type()) + " hits you for " + std::to_string(e.attackPower()) + ".");
            if (!player_.alive())
            {
                gameOver_ = true; addLog("You died.");
                return;
            }
            continue;
        }

        const int chebyshev = std::max(std::abs(dx), std::abs(dy));
        if (chebyshev > e.sight())
            continue;

        const int sx = sgn(dx), sy = sgn(dy);
        auto tryStep = [&](int mx, int my) {
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
    for (size_t i = 0; i < enemies_.size();) {
        if (!enemies_[i].alive())
            enemies_.erase(enemies_.begin() + i);
        else ++i;
    }
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