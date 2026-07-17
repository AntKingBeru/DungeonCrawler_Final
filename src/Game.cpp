#include "Game.h"
#include "ConfigUtil.h"
#include "ParserFactory.h"
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <queue>
#include <climits>

int Game::sellValue(const Item& it)
{
    const int base = it.atk * 3 + it.def * 3 + it.hp + it.heal;
    return std::max(1, base / 2);
}

void Game::readMeta(const ConfigData& data)
{
    levelName_ = cfg::require(data, "meta", "name");
    nextLevel_ = cfg::strOr(data, "meta", "next", "");
    enemyScale_ = cfg::floatOr(data, "meta", "enemy_scale", 1.0f);
}

void Game::buildStatic(const ConfigData& data)
{
    map_.loadFrom(data);
    lootTables_.clear();
    shopStock_.clear();
    shopkeeper_ = {};

    if (auto it = data.find("shopkeeper"); it != data.end())
        shopkeeper_ = { cfg::requireInt(data, "shopkeeper", "x"),
                        cfg::requireInt(data, "shopkeeper", "y"), true };

    if (auto it = data.find("shop"); it != data.end())
        for (const auto& [id, spec] : it->second)
        {
            std::istringstream ss(spec);
            int price; if (!(ss >> price))
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
            double chance; if (!(ss >> chance))
                continue;
            table.add(item, chance);
        }
        lootTables_[name] = table;
    }
}

Enemy* Game::spawnEnemy(const std::string& type, int x, int y)
{
    std::optional<Enemy> made = Enemy::create(type, x, y);
    if (!made)
        return nullptr;
    enemies_.push_back(std::move(*made));
    Enemy* e = &enemies_.back();
    e->scaleStats(enemyScale_);
    return e;
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
        std::string type; int x, y;
        if (!(ss >> type >> x >> y))
            continue;
        Enemy* e = spawnEnemy(type, x, y);
        if (!e)
            continue;
        int hp;
        if (ss >> hp)
            e->setHp(hp);
        std::string flag;
        if (ss >> flag && flag == "boss")
            e->setBoss(true);
    }
}

void Game::buildBoss(const ConfigData& data)
{
    auto it = data.find("boss");
    if (it == data.end())
        return;
    for (const auto& [id, spec] : it->second)
    {
        std::istringstream ss(spec);
        std::string type;
        int x, y;
        if (!(ss >> type >> x >> y))
            continue;
        Enemy* e = spawnEnemy(type, x, y);
        if (!e)
            continue;
        e->setBoss(true);
        int hp; if (ss >> hp)
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
    parseChestsInto(data, section, chests_);
}

void Game::parseChestsInto(const ConfigData& data, const std::string& section, std::vector<Chest>& out)
{
    out.clear();
    auto it = data.find(section);
    if (it == data.end())
        return;
    for (const auto& [id, spec] : it->second)
    {
        std::istringstream ss(spec);
        int x, y;
        std::string table;
        if (ss >> x >> y >> table)
            out.push_back({ x, y, table });
    }
}

void Game::buildDoors(const ConfigData& data, const std::string& section)
{
    doors_.clear();
    auto it = data.find(section);
    if (it == data.end())
        return;
    for (const auto& [id, spec] : it->second)
    {
        std::istringstream ss(spec);
        int x, y;
        std::string area;
        if (ss >> x >> y >> area)
            doors_.push_back({ x, y, area });
    }
}

const Door* Game::doorAt(int x, int y) const
{
    for (const auto& d : doors_)
        if (d.x == x && d.y == y)
            return &d;
    return nullptr;
}

bool Game::hasLivingBoss() const
{
    for (const auto& e : enemies_)
        if (e.isBoss() && e.alive())
            return true;
    return false;
}

void Game::onBossDefeated()
{
    if (bossDefeated_ || hasLivingBoss())
        return;
    const int n = static_cast<int>(pendingBossChests_.size());
    for (const auto& c : pendingBossChests_)
        chests_.push_back(c);
    pendingBossChests_.clear();
    bossDefeated_ = true;
    player_.addChestKeys(n);
    addLog("The guardian's hoard appears! (+" + std::to_string(n) + " chest keys)");
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
    readMeta(data);

    buildStatic(data);
    player_.loadFrom(data);
    buildEnemies(data);
    buildBoss(data);
    buildGround(data, "items");
    buildChests(data, "chests");
    parseChestsInto(data, "boss_chests", pendingBossChests_);
	buildDoors(data, "doors");
    bossDefeated_ = false;
    resetRuntime();
    addLog("Entered " + levelName_ + ".");
}

void Game::descend()
{
    levelPath_ = nextLevel_;
    ConfigData data = makeParser(levelPath_)->parse(levelPath_);
    readMeta(data);

    buildStatic(data);
    player_.moveToStart(data);
    buildEnemies(data);
    buildBoss(data);
    buildGround(data, "items");
    buildChests(data, "chests");
    parseChestsInto(data, "boss_chests", pendingBossChests_);
    buildDoors(data, "doors");
    bossDefeated_ = false;
    shopOpen_ = false;
    addLog("Descended to " + levelName_ + ".");
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
    readMeta(level);

    buildStatic(level);
    player_.readState(save);
    buildEnemies(save);
    buildGround(save, "ground");
    buildChests(save, "chests");
	buildDoors(save, "doors");
    bossDefeated_ = (cfg::strOr(save, "save", "boss_defeated", "0") == "1");
    if (bossDefeated_)
        pendingBossChests_.clear();
    else
        parseChestsInto(level, "boss_chests", pendingBossChests_);
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
    d["save"]["boss_defeated"] = bossDefeated_ ? "1" : "0";

    player_.writeState(d);

    int n = 0;
    for (const auto& e : enemies_)
        if (e.alive())
        {
            std::string line = e.type() + " " + std::to_string(e.x()) + " " + std::to_string(e.y())
                + " " + std::to_string(e.hp());
            if (e.isBoss())
                line += " boss";
            d["enemies"]["e" + std::to_string(n++)] = line;
        }

    n = 0;
    for (const auto& g : ground_)
        d["ground"]["g" + std::to_string(n++)] =
        std::to_string(g.x) + " " + std::to_string(g.y) + " " + itemToBody(g.item);

    n = 0;
    for (const auto& c : chests_)
        d["chests"]["c" + std::to_string(n++)] =
        std::to_string(c.x) + " " + std::to_string(c.y) + " " + c.table;

    n = 0;
    for (const auto& dr: doors_)
        d["doors"]["d" + std::to_string(n++)] = 
		std::to_string(dr.x) + " " + std::to_string(dr.y) + " " + dr.area;

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
    if (doorAt(x, y))
        return true;
    return false;
}

void Game::movePlayer(int dx, int dy)
{
    if (complete_ || gameOver_ || player_.isMoving())
        return;
    const int nx = player_.x() + dx, ny = player_.y() + dy;

    if (const Door* d = doorAt(nx, ny))
    {
        if (!player_.useDoorKey())
        {
            addLog("The way is locked. You need a key.");
            return;
        }
        const std::string area = d->area;
        doors_.erase(std::remove_if(doors_.begin(), doors_.end(),
            [&](const Door& e)
            {
                return e.area == area;
            }
        ), doors_.end());
        addLog("You unlocked the " + area + "door.");
    }
    if (blocked(nx, ny))
        return;

    player_.setTile(nx, ny);
    tryPickUp(nx, ny);
    if (map_.isExit(nx, ny))
    {
        if (hasLivingBoss())
        {
            addLog("The exit is sealed until the guardian falls.");
            enemyTurn();
            return;
        }
        if (!nextLevel_.empty())
            descend();
        else
        {
            complete_ = true;
            addLog("You escaped the dungeon!");
        }
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
        addLog("You hit the " + e->name() + " for " + std::to_string(dmg) + ".");
        if (!e->alive())
        {
            const int xp = e->maxHp() + e->attackPower();
            if (e->isBoss())
            {
                addLog("The " + e->name() + " guardian falls!");
                onBossDefeated();
            }
            else
            {
                addLog(e->name() + " slain!");
                dropLoot(e->type(), e->x(), e->y());
            }
            const int gained = player_.gainExp(xp);
            addLog("Gained " + std::to_string(xp) + " EXP.");
            if (gained)
                addLog("Level up! You are now level " + std::to_string(player_.level()) + "!");
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
    if (!player_.useChestKey())
    {
        addLog("The chest is locked. You need a chest key.");
        return;
    }
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
        else if (item.isDoorKey())
        {
            player_.addDoorKeys(1);
            addLog("Found a door key!");
        }
        else if (item.isChestKey())
        {
			player_.addChestKeys(1);
			addLog("Found a chest key!");
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
    if (e.item.isDoorKey())
    {
        player_.spendGold(e.price);
        player_.addDoorKeys(1);
        addLog("Bought a door key.");
        return;
    }
    if (e.item.isChestKey())
    {
        player_.spendGold(e.price);
        player_.addChestKeys(1);
        addLog("Bought a chest key.");
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

bool Game::hasLineOfSight(int x0, int y0, int x1, int y1) const
{
    auto blocksSight = [&](int x, int y)
        {
            if (map_.isWall(x, y))
                return true;
            for (const auto& c : chests_)
                if (c.x == x && c.y == y)
                    return true;
            if (doorAt(x, y))
                return true;
            return false;
        };
    int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int err = dx - dy, cx = x0, cy = y0;
    while (true)
    {
        if (!(cx == x0 && cy == y0) && !(cx == x1 && cy == y1) && blocksSight(cx, cy))
            return false;
        if (cx == x1 && cy == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            cx += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            cy += sy;
        }
    }
    return true;
}

bool Game::detects(const Enemy& e) const
{
    const int dx = std::abs(player_.x() - e.x()), dy = std::abs(player_.y() - e.y());
    if (std::max(dx, dy) > e.sight())
        return false;
    return hasLineOfSight(e.x(), e.y(), player_.x(), player_.y());
}

bool Game::passableForPath(int x, int y, const Enemy* self, int goalX, int goalY) const
{
    if (x == goalX && y == goalY)
        return true;
    if (map_.isWall(x, y))
        return false;
    for (const auto& e : enemies_)
        if (&e != self && e.alive() && e.x() == x && e.y() == y)
            return false;
    for (const auto& c : chests_)
        if (c.x == x && c.y == y)
            return false;
    if (shopkeeper_.exists && shopkeeper_.x == x && shopkeeper_.y == y)
        return false;
    if (doorAt(x, y))
        return false;
    if (x == player_.x() && y == player_.y())
        return false;
    return true;
}

std::pair<int, int> Game::aStarStep(const Enemy* self, int tx, int ty) const
{
    const int W = map_.width(), H = map_.height();
    const int sx = self->x(), sy = self->y();
    if (sx == tx && sy == ty)
        return { 0, 0 };
    auto idx = [&](int x, int y)
        {
            return y * W + x;
        };
    auto h = [&](int x, int y)
        {
            return std::abs(x - tx) + std::abs(y - ty);
        };
    std::vector<int> came(W * H, -1);
    std::vector<int> g(W * H, INT_MAX);
    std::vector<char> closed(W * H, 0);
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
        std::greater<std::pair<int, int>>> open;
    g[idx(sx, sy)] = 0;
    open.push({ h(sx, sy), idx(sx, sy) });
    const int dxs[4] = { 1,-1,0,0 }, dys[4] = { 0,0,1,-1 };
    bool found = false;
    while (!open.empty())
    {
        int cur = open.top().second;
        open.pop();
        if (closed[cur])
            continue;
        closed[cur] = 1;
        int cx = cur % W, cy = cur / W;
        if (cx == tx && cy == ty)
        {
            found = true;
            break;
        }
        for (int d = 0; d < 4; ++d)
        {
            int nx = cx + dxs[d], ny = cy + dys[d];
            if (nx < 0 || ny < 0 || nx >= W || ny >= H)
                continue;
            if (!passableForPath(nx, ny, self, tx, ty))
                continue;
            int ng = g[cur] + 1;
            if (ng < g[idx(nx, ny)])
            {
                g[idx(nx, ny)] = ng;
                came[idx(nx, ny)] = cur;
                open.push({ ng + h(nx, ny), idx(nx, ny) });
            }
        }
    }
    if (!found)
        return { 0, 0 };
    int c = idx(tx, ty), prev = came[c];
    if (prev == -1)
        return { 0, 0 };
    while (prev != idx(sx, sy))
    {
        c = prev; prev = came[c];
        if (prev == -1)
            return { 0, 0 };
    }
    return { c % W - sx, c / W - sy };
}

std::pair<int, int> Game::roamStep(Enemy* self)
{
    const int dxs[4] = { 1,-1,0,0 }, dys[4] = { 0,0,1,-1 };
    std::vector<std::pair<int, int>> cands;
    for (int d = 0; d < 4; ++d)
    {
        int nx = self->x() + dxs[d], ny = self->y() + dys[d];
        if (wallOrEnemy(nx, ny, self))
            continue;
        if (nx == player_.x() && ny == player_.y())
            continue;
        if (std::max(std::abs(nx - self->spawnX()), std::abs(ny - self->spawnY())) > self->roamRange())
            continue;
        cands.push_back({ dxs[d], dys[d] });
    }
    if (cands.empty())
        return { 0, 0 };
    std::uniform_int_distribution<int> pick(0, static_cast<int>(cands.size()));
    int r = pick(rng_);
    if (r >= static_cast<int>(cands.size()))
        return { 0, 0 };
    return cands[r];
}

void Game::enemyTurn()
{
    constexpr int AGGRO_TURNS = 6;
    for (auto& e : enemies_)
    {
        if (!e.alive())
            continue;
        const int px = player_.x(), py = player_.y();
        const int manhattan = std::abs(px - e.x()) + std::abs(py - e.y());

        if (manhattan == 1)
        {
            const int dmg = std::max(1, e.attackPower() - player_.defense());
            player_.takeDamage(dmg);
            e.setAggro(AGGRO_TURNS); e.setLastSeen(px, py);
            addLog(e.name() + " hits you for " + std::to_string(dmg) + ".");
            if (!player_.alive())
            {
                gameOver_ = true;
                addLog("You died.");
                return;
            }
            continue;
        }

        const bool see = detects(e);
        if (see)
        {
            e.setAggro(AGGRO_TURNS);
            e.setLastSeen(px, py);
        }

        std::pair<int, int> step{ 0, 0 };
        if (e.aggro() > 0)
        {
            const int tx = e.lastSeenX(), ty = e.lastSeenY();
            if (e.x() == tx && e.y() == ty)
                e.setAggro(0);
            else
                step = aStarStep(&e, tx, ty);
            if (!see)
                e.setAggro(e.aggro() - 1);
        }
        else
        {
            const int fromSpawn = std::max(std::abs(e.x() - e.spawnX()), std::abs(e.y() - e.spawnY()));
            if (fromSpawn > e.roamRange())
                step = aStarStep(&e, e.spawnX(), e.spawnY());
            else if (!e.isBoss() && e.roamRange() > 0)
                step = roamStep(&e);
        }
        if (step.first || step.second)
            e.setTile(e.x() + step.first, e.y() + step.second);
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
    for (const auto& c : chests_)
        if (c.x == x && c.y == y)
            return true;
    if (shopkeeper_.exists && shopkeeper_.x == x && shopkeeper_.y == y)
        return true;
    if (doorAt(x, y))
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