#include "Renderer.h"
#include "Game.h"
#include "raylib.h"
#include <string>
#include <vector>
#include <algorithm>
#include <utility>

namespace
{
    void originOf(const Map& map, int& ox, int& oy)
    {
        ox = std::max(0, (SCREEN_W - map.width() * TILE_SIZE) / 2);
        oy = std::max(0, (VIEW_H - map.height() * TILE_SIZE) / 2);
    }

    constexpr int BOX = 42, GAP = 8, LABELW = 90;
    struct Box
    {
        int x, y, w, h;
        bool has(int px, int py) const
        {
            return px >= x && px < x + w && py >= y && py < y + h;
        }
    };
    struct GearBox
    {
        Box box;
        ItemSlot slot;
        int sub;
    };
    struct Single
    {
        ItemSlot slot;
        const char* label;
    };
    const Single kSingles[8] =
    {
        {ItemSlot::Weapon,"Weapon"},
        {ItemSlot::Shield,"Shield"},
        {ItemSlot::Helmet,"Helmet"},
        {ItemSlot::Cape,"Cape"},
        {ItemSlot::Armor,"Armor"},
        {ItemSlot::Gloves,"Gloves"},
        {ItemSlot::Boots,"Boots"},
        {ItemSlot::Amulet,"Amulet"}
    };
    constexpr int SINGLES = 8;

    void invLayout(int screenW, std::vector<GearBox>& gear, std::vector<Box>& bag,
        int& panelX, int& panelY, int& ringY, int& bagX, int& sy)
    {
        panelX = 40;
        panelY = 30;
        const int sx = panelX + 24;
        sy = panelY + 60;
        gear.clear();
        bag.clear();
        for (int i = 0; i < SINGLES; ++i)
            gear.push_back({ { sx + LABELW, sy + i * (BOX + GAP), BOX, BOX }, kSingles[i].slot, 0 });
        ringY = sy + SINGLES * (BOX + GAP) + 26;
        for (int j = 0; j < 8; ++j)
            gear.push_back({ { sx + j * (BOX + GAP), ringY, BOX, BOX }, ItemSlot::Ring, j });
        bagX = screenW / 2 + 20;
        for (int k = 0; k < 27; ++k)
            bag.push_back({ bagX + (k % 9) * (BOX + GAP), sy + (k / 9) * (BOX + GAP), BOX, BOX });
    }

    void shopLayout(int screenW, int stockCount,
        std::vector<Box>& stock, std::vector<Box>& bag, Box& close,
        int& panelX, int& panelY, int& bagX, int& sy)
    {
        panelX = 40;
        panelY = 30;
        sy = panelY + 74;
        const int gx = panelX + 24;
        close = { screenW - panelX - 110, panelY + 14, 90, 30 };
        stock.clear();
        bag.clear();
        for (int i = 0; i < stockCount; ++i)
            stock.push_back({ gx, sy + i * (BOX + GAP), BOX, BOX });
        bagX = screenW / 2 + 40;
        for (int k = 0; k < 27; ++k)
            bag.push_back({ bagX + (k % 9) * (BOX + GAP), sy + (k / 9) * (BOX + GAP), BOX, BOX });
    }

    Color slotColor(ItemSlot s)
    {
        switch (s)
        {
            case ItemSlot::Weapon:
                return ORANGE;
            case ItemSlot::Shield:
                return Color{ 70,130,180,255 };
            case ItemSlot::Helmet:
                return SKYBLUE;
            case ItemSlot::Cape:
                return VIOLET;
            case ItemSlot::Armor:
                return LIGHTGRAY;
            case ItemSlot::Gloves:
                return BEIGE;
            case ItemSlot::Boots:
                return BROWN;
            case ItemSlot::Amulet:
                return GOLD;
            case ItemSlot::Ring:
                return YELLOW;
        }
        return GRAY;
    }
    Color itemColor(const Item& it)
    {
        if (it.isKey())
            return it.isDoorKey() ? GOLD : Color{ 190, 150, 70, 255 };
        return it.isPotion() ? PINK : slotColor(it.slot);
    }

    Color enemyColor(const std::string& t)
    {
        if (t == "hobgoblin")
            return Color{ 120, 60,130,255 };
        if (t == "knight")
            return Color{ 70, 80,110,255 };
        if (t == "archer")
            return Color{ 60,110, 95,255 };
        if (t == "mage")
            return Color{ 120, 70,170,255 };
        if (t == "golem")
            return Color{ 85, 85,100,255 };
        if (t == "wyvern")
            return Color{ 95, 45,120,255 };
        if (t == "lich")
            return Color{ 140,125,180,255 };
        return Color{ 95, 75,115,255 };
    }
    float enemyRadius(const std::string& t)
    {
        if (t == "golem")
            return 0.48f;
        if (t == "wyvern")
            return 0.46f;
        if (t == "lich")
            return 0.44f;
        if (t == "hobgoblin")
            return 0.42f;
        return 0.35f;
    }

    void drawHpBar(int tileScreenX, int tileScreenY, int hp, int maxHp)
    {
        if (maxHp <= 0)
            return;
        const int w = static_cast<int>(TILE_SIZE * 0.7f), h = 4;
        const int x = tileScreenX + (TILE_SIZE - w) / 2, y = tileScreenY + 2;
        DrawRectangle(x, y, w, h, MAROON);
        DrawRectangle(x, y, static_cast<int>(w * (static_cast<float>(hp) / maxHp)), h, LIME);
    }

    void drawCell(const Box& b, const Item* item)
    {
        if (item)
        {
            DrawRectangle(b.x, b.y, b.w, b.h, itemColor(*item));
            char c[2] = { item->name.empty() ? '?' : item->name[0], 0 };
            DrawText(c, b.x + b.w / 2 - 5, b.y + b.h / 2 - 8, 18, BLACK);
        }
        else
        {
            DrawRectangle(b.x, b.y, b.w, b.h, Color{ 40,40,50,255 });
        }
        DrawRectangleLines(b.x, b.y, b.w, b.h, GRAY);
    }

    const Color TEAL{ 0,170,160,255 };

    std::string signedStr(int v)
    {
        return (v > 0 ? "+" : "") + std::to_string(v);
    }

    void drawItemTooltip(const Item& it, bool equipped, const Inventory& inv,
        int mx, int my, int screenW, int screenH, int sellFor = -1)
    {
        std::vector<std::pair<std::string, Color>> lines;
        lines.push_back({ it.name, RAYWHITE });
        if (it.isKey())
        {
            lines.push_back({ it.isDoorKey() ? "Door Key" : "Chest Key", Color{230,200,90,255} });
            lines.push_back({ it.isDoorKey() ? "Unlocks a locked area" : "Opens a chest", LIGHTGRAY });
        }
        else if (it.isPotion())
        {
            lines.push_back({ "Consumable", LIGHTGRAY });
            lines.push_back({ "Restores " + std::to_string(it.heal) + " HP", Color{120,220,120,255} });
        }
        else
        {
            lines.push_back({ std::string(slotName(it.slot)), LIGHTGRAY });
            if (it.atk)
                lines.push_back({ "ATK " + signedStr(it.atk), RAYWHITE });
            if (it.def)
                lines.push_back({ "DEF " + signedStr(it.def), RAYWHITE });
            if (it.hp)
                lines.push_back({ "HP  " + signedStr(it.hp),  RAYWHITE });
            if (!it.atk && !it.def && !it.hp)
                lines.push_back({ "No bonuses", LIGHTGRAY });
            if (equipped)
            {
                lines.push_back({ "(equipped)", GOLD });
            }
            else if (it.slot != ItemSlot::Ring)
            {
                auto e = inv.equipped().find(it.slot);
                if (e != inv.equipped().end() && !e->second.empty())
                {
                    const Item& cur = e->second[0];
                    lines.push_back({ "vs " + cur.name + ":", LIGHTGRAY });
                    auto delta = [&](const char* n, int d)
                        {
                            Color c = d > 0 ? Color{ 120,220,120,255 } : d < 0 ? Color{ 220,110,110,255 } : GRAY;
                            lines.push_back({ std::string(n) + " " + signedStr(d), c });
                        };
                    delta("ATK", it.atk - cur.atk); delta("DEF", it.def - cur.def); delta("HP ", it.hp - cur.hp);
                }
                else
                {
                    lines.push_back({ "(slot empty)", LIGHTGRAY });
                }
            }
        }
        if (sellFor > 0)
            lines.push_back({ "Sells for " + std::to_string(sellFor) + " gold", GOLD });

        int wpx = 0;
        for (auto& ln : lines)
            wpx = std::max(wpx, MeasureText(ln.first.c_str(), 16));
        const int pad = 10, lh = 20;
        const int w = wpx + 2 * pad, h = static_cast<int>(lines.size()) * lh + 2 * pad;
        int x = mx + 16, y = my + 16;
        if (x + w > screenW)
            x = mx - w - 8;
        if (y + h > screenH)
            y = screenH - h - 4;
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
        DrawRectangle(x, y, w, h, Color{ 20,20,28,245 });
        DrawRectangleLines(x, y, w, h, RAYWHITE);
        int ty = y + pad;
        for (auto& ln : lines)
        {
            DrawText(ln.first.c_str(), x + pad, ty, 16, ln.second); ty += lh;
        }
    }

}

void Renderer::draw(const Game& game, bool showInventory) const
{
    const Map& map = game.map();
    const Player& player = game.player();
    const int screenW = SCREEN_W;
    const int screenH = VIEW_H;
    int ox, oy;
    originOf(map, ox, oy);
    const bool shopOpen = game.isShopOpen();

    DrawRectangle(0, 0, screenW, screenH, Color{ 10,10,14,255 });
    DrawRectangle(ox, oy, map.width() * TILE_SIZE, map.height() * TILE_SIZE, DARKGRAY);

    for (int y = 0; y < map.height(); ++y)
        for (int x = 0; x < map.width(); ++x)
        {
            const int px = ox + x * TILE_SIZE, py = oy + y * TILE_SIZE;
            if (map.isWall(x, y))
                DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, RAYWHITE);
            else if (map.isExit(x, y))
                DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, GREEN);
        }

    for (const auto& gi : game.groundItems())
        DrawRectangle(ox + gi.x * TILE_SIZE + TILE_SIZE / 4, oy + gi.y * TILE_SIZE + TILE_SIZE / 4,
            TILE_SIZE / 2, TILE_SIZE / 2, itemColor(gi.item));

    for (const auto& c : game.chests())
    {
        const int px = ox + c.x * TILE_SIZE, py = oy + c.y * TILE_SIZE;
        DrawRectangle(px + TILE_SIZE / 6, py + TILE_SIZE / 4, TILE_SIZE * 2 / 3, TILE_SIZE / 2, BROWN);
        DrawRectangle(px + TILE_SIZE / 6, py + TILE_SIZE / 4, TILE_SIZE * 2 / 3, TILE_SIZE / 8, GOLD);
        DrawRectangleLines(px + TILE_SIZE / 6, py + TILE_SIZE / 4, TILE_SIZE * 2 / 3, TILE_SIZE / 2, BLACK);
    }

    for (const auto& d : game.doors())
    {
        const int px = ox + d.x * TILE_SIZE, py = oy + d.y * TILE_SIZE;
        DrawRectangle(px + 2, py + 2, TILE_SIZE - 4, TILE_SIZE - 4, Color{ 110, 60, 30, 255 });
        DrawRectangle(px + TILE_SIZE/2 - 3, py + TILE_SIZE/2 - 7, 6, 14, GOLD);
        DrawRectangleLines(px + 2, py + 2, TILE_SIZE - 4, TILE_SIZE - 4, Color{ 60, 30, 15, 255 });
    }

    if (game.shopkeeper().exists)
    {
        const int px = ox + game.shopkeeper().x * TILE_SIZE, py = oy + game.shopkeeper().y * TILE_SIZE;
        DrawCircle(px + TILE_SIZE / 2, py + TILE_SIZE / 2, TILE_SIZE * 0.38f, TEAL);
        DrawCircle(px + TILE_SIZE / 2, py + TILE_SIZE / 2, TILE_SIZE * 0.16f, GOLD);
        DrawText("$", px + TILE_SIZE / 2 - 4, py + TILE_SIZE / 2 - 8, 18, BLACK);
    }

    for (const auto& e : game.enemies())
    {
        const int ex = ox + static_cast<int>(e.visualX() * TILE_SIZE);
        const int ey = oy + static_cast<int>(e.visualY() * TILE_SIZE);
        const int cx = ex + TILE_SIZE / 2, cy = ey + TILE_SIZE / 2;
        DrawCircle(cx, cy, TILE_SIZE * enemyRadius(e.type()), enemyColor(e.type()));
        if (e.isBoss())
            DrawCircleLines(cx, cy, TILE_SIZE * enemyRadius(e.type()) + 3, GOLD);
        drawHpBar(ex, ey, e.hp(), e.maxHp());
    }
    {
        const int pxp = ox + static_cast<int>(player.visualX() * TILE_SIZE);
        const int pyp = oy + static_cast<int>(player.visualY() * TILE_SIZE);
        DrawCircle(pxp + TILE_SIZE / 2, pyp + TILE_SIZE / 2, TILE_SIZE * 0.35f, YELLOW);
        drawHpBar(pxp, pyp, player.hp(), player.maxHp());
    }

    DrawRectangle(0, screenH, screenW, HUD_HEIGHT, BLACK);
    std::string statLine = TextFormat("HP %d/%d   ATK %d   DEF %d   GOLD %d   Keys D:%d C:%d",
        player.hp(), player.maxHp(), player.attackPower(), player.defense(), game.gold(),
        player.doorKeys(), player.chestKeys());
    DrawText(statLine.c_str(), 10, screenH + 8, 20,
        player.hp() > player.maxHp() / 4 ? RAYWHITE : RED);
    DrawText(TextFormat("Lv %d   EXP %d/%d", player.level(), player.exp(), player.expToNext()),
        10 + MeasureText(statLine.c_str(), 20) + 28, screenH + 8, 20, Color{ 210,205,120,255 });
    DrawText("[I] Inventory", screenW - 150, screenH + 8, 18, LIGHTGRAY);
    {
        const std::string& fl = game.levelName();
        DrawText(fl.c_str(), screenW - MeasureText(fl.c_str(), 18) - 10, screenH + 34, 18, GOLD);
    }
    const auto& log = game.log();
    for (int i = 0; i < 3 && i < static_cast<int>(log.size()); ++i)
        DrawText(log[log.size() - 1 - i].c_str(), 10, screenH + 34 + i * 18, 16, LIGHTGRAY);

    if (showInventory && !shopOpen)
    {
        DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.75f));
        int panelX, panelY, ringY, bagX, sy2; std::vector<GearBox> gear; std::vector<Box> bag;
        invLayout(screenW, gear, bag, panelX, panelY, ringY, bagX, sy2);
        DrawRectangle(panelX, panelY, screenW - 2 * panelX, screenH - 2 * panelY, Color{ 25,25,35,255 });
        DrawRectangleLines(panelX, panelY, screenW - 2 * panelX, screenH - 2 * panelY, RAYWHITE);
        DrawText("INVENTORY", panelX + 24, panelY + 16, 22, RAYWHITE);
        DrawText("Equipment", panelX + 24, sy2 - 24, 16, LIGHTGRAY);
        DrawText("Backpack", bagX, sy2 - 24, 16, LIGHTGRAY);

        const Inventory& inv = player.inventory();
        for (int i = 0; i < SINGLES; ++i)
        {
            DrawText(kSingles[i].label, panelX + 24, gear[i].box.y + 12, 16, LIGHTGRAY);
            auto it = inv.equipped().find(kSingles[i].slot);
            const Item* item = (it != inv.equipped().end() && !it->second.empty()) ? &it->second[0] : nullptr;
            drawCell(gear[i].box, item);
        }
        DrawText("Rings", panelX + 24, ringY - 22, 16, LIGHTGRAY);
        for (int j = 0; j < 8; ++j)
        {
            auto it = inv.equipped().find(ItemSlot::Ring);
            const Item* item = (it != inv.equipped().end() && j < static_cast<int>(it->second.size())) ? &it->second[j] : nullptr;
            drawCell(gear[SINGLES + j].box, item);
        }
        for (int k = 0; k < 27; ++k)
        {
            const auto& slot = inv.storage()[k];
            drawCell(bag[k], slot ? &*slot : nullptr);
        }
        DrawText("Click backpack: equip gear / drink potion.  Click worn gear: unequip.  [I] close.",
            panelX + 24, screenH - panelY - 26, 16, LIGHTGRAY);

        Vector2 mp = GetMousePosition();
        const int mx = static_cast<int>(mp.x), my = static_cast<int>(mp.y);
        const Item* hover = nullptr; bool hoverEquipped = false;
        for (const auto& g : gear)
        {
            if (!g.box.has(mx, my))
                continue;
            auto e = inv.equipped().find(g.slot);
            if (e != inv.equipped().end() && g.sub < static_cast<int>(e->second.size()))
            {
                hover = &e->second[g.sub]; hoverEquipped = true;
            }
        }
        if (!hover)
            for (int k = 0; k < 27; ++k)
                if (bag[k].has(mx, my) && inv.storage()[k])
                {
                    hover = &*inv.storage()[k];
                    break;
                }
        if (hover)
            drawItemTooltip(*hover, hoverEquipped, inv, mx, my, screenW, screenH);
    }

    if (shopOpen)
    {
        DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.8f));
        int panelX, panelY, bagX, sy;
        std::vector<Box> stock, bag;
        Box close;
        shopLayout(screenW, static_cast<int>(game.shopStock().size()), stock, bag, close, panelX, panelY, bagX, sy);
        DrawRectangle(panelX, panelY, screenW - 2 * panelX, screenH - 2 * panelY, Color{ 20,30,30,255 });
        DrawRectangleLines(panelX, panelY, screenW - 2 * panelX, screenH - 2 * panelY, TEAL);
        DrawText("SHOPKEEPER", panelX + 24, panelY + 16, 22, TEAL);
        DrawText(TextFormat("Your gold: %d", game.gold()), panelX + 220, panelY + 20, 18, GOLD);
        DrawRectangle(close.x, close.y, close.w, close.h, Color{ 60,30,30,255 });
        DrawRectangleLines(close.x, close.y, close.w, close.h, RAYWHITE);
        DrawText("Close", close.x + 18, close.y + 7, 18, RAYWHITE);

        DrawText("For sale (click to buy)", panelX + 24, sy - 24, 16, LIGHTGRAY);
        const auto& shop = game.shopStock();
        for (size_t i = 0; i < shop.size(); ++i)
        {
            drawCell(stock[i], &shop[i].item);
            const bool afford = game.gold() >= shop[i].price;
            DrawText(shop[i].item.name.c_str(), stock[i].x + BOX + 10, stock[i].y + 4, 18, RAYWHITE);
            DrawText(TextFormat("%d gold", shop[i].price), stock[i].x + BOX + 10, stock[i].y + 24, 16,
                afford ? GOLD : GRAY);
        }

        DrawText("Backpack (click to sell)", bagX, sy - 24, 16, LIGHTGRAY);
        const Inventory& inv = player.inventory();
        for (int k = 0; k < 27; ++k)
        {
            const auto& slot = inv.storage()[k];
            drawCell(bag[k], slot ? &*slot : nullptr);
        }
        DrawText("Selling pays about half an item's worth.  Click Close or press [I] to leave.",
            panelX + 24, screenH - panelY - 26, 16, LIGHTGRAY);

        Vector2 mp = GetMousePosition();
        const int mx = static_cast<int>(mp.x), my = static_cast<int>(mp.y);
        const Item* hover = nullptr; int sellFor = -1;
        for (size_t i = 0; i < shop.size(); ++i)
            if (stock[i].has(mx, my))
            {
                hover = &shop[i].item;
                break;
            }
        if (!hover)
            for (int k = 0; k < 27; ++k)
                if (bag[k].has(mx, my) && inv.storage()[k])
                {
                    hover = &*inv.storage()[k];
                    sellFor = Game::sellValue(*hover);
                    break;
                }
        if (hover)
            drawItemTooltip(*hover, false, inv, mx, my, screenW, screenH, sellFor);
    }

    if (!showInventory && !shopOpen && game.isGameOver())
    {
        DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.6f));
        const char* m = "YOU DIED";
        DrawText(m, (screenW - MeasureText(m, 48)) / 2, screenH / 2 - 24, 48, RED);
    }
    else if (!showInventory && !shopOpen && game.isComplete() && !player.isMoving())
    {
        DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.6f));
        const char* m = "YOU ESCAPED!";
        DrawText(m, (screenW - MeasureText(m, 40)) / 2, screenH / 2 - 40, 40, GREEN);
        const char* s = "Press Esc for menu";
        DrawText(s, (screenW - MeasureText(s, 20)) / 2, screenH / 2 + 10, 20, RAYWHITE);
    }
}

InvHit Renderer::hitTestInventory(const Game& game, int mx, int my) const
{
    const int screenW = SCREEN_W;
    int panelX, panelY, ringY, bagX, sy2;
    std::vector<GearBox> gear;
    std::vector<Box> bag;
    invLayout(screenW, gear, bag, panelX, panelY, ringY, bagX, sy2);
    const Inventory& inv = game.player().inventory();
    for (const auto& g : gear)
        if (g.box.has(mx, my) && g.sub < inv.equippedCount(g.slot))
            return InvHit{ InvHit::Gear, -1, g.slot, g.sub };
    for (int k = 0; k < 27; ++k)
        if (bag[k].has(mx, my) && inv.storage()[k].has_value())
            return InvHit{ InvHit::Storage, k, ItemSlot::Weapon, -1 };
    return InvHit{};
}

ShopHit Renderer::hitTestShop(const Game& game, int mx, int my) const
{
    const int screenW = SCREEN_W;
    int panelX, panelY, bagX, sy;
    std::vector<Box> stock, bag;
    Box close;
    shopLayout(screenW, static_cast<int>(game.shopStock().size()), stock, bag, close, panelX, panelY, bagX, sy);
    if (close.has(mx, my))
        return ShopHit{ ShopHit::Close, -1 };
    for (size_t i = 0; i < stock.size(); ++i)
        if (stock[i].has(mx, my))
            return ShopHit{ ShopHit::Buy, static_cast<int>(i) };
    const Inventory& inv = game.player().inventory();
    for (int k = 0; k < 27; ++k)
        if (bag[k].has(mx, my) && inv.storage()[k].has_value())
            return ShopHit{ ShopHit::Sell, k };
    return ShopHit{};
}

void Renderer::screenToTile(const Game& game, int mx, int my, int& tileX, int& tileY) const
{
    int ox, oy;
    originOf(game.map(), ox, oy);
    const int rx = mx - ox, ry = my - oy;
    if (rx < 0 || ry < 0)
    {
        tileX = tileY = -1;
        return;
    }
    tileX = rx / TILE_SIZE;
    tileY = ry / TILE_SIZE;
}