#include "Renderer.h"
#include "Game.h"
#include "raylib.h"
#include <string>
#include <vector>
#include <algorithm>

namespace
{
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
    const Single kSingles[7] =
    {
        {ItemSlot::Weapon,"Weapon"},
        {ItemSlot::Helmet,"Helmet"},
        {ItemSlot::Cape,"Cape"},
        {ItemSlot::Armor,"Armor"},
        {ItemSlot::Gloves,"Gloves"},
        {ItemSlot::Boots,"Boots"},
        {ItemSlot::Amulet,"Amulet"}
    };

    void invLayout(int screenW, std::vector<GearBox>& gear, std::vector<Box>& bag,
        int& panelX, int& panelY, int& ringY, int& bagX, int& sy)
    {
        panelX = 40; panelY = 30;
        const int sx = panelX + 24;
        sy = panelY + 60;
        gear.clear(); bag.clear();
        for (int i = 0; i < 7; ++i)
            gear.push_back({ { sx + LABELW, sy + i * (BOX + GAP), BOX, BOX }, kSingles[i].slot, 0 });
        ringY = sy + 7 * (BOX + GAP) + 26;
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
        panelX = 40; panelY = 30;
        sy = panelY + 74;
        const int gx = panelX + 24;
        close = { screenW - panelX - 110, panelY + 14, 90, 30 };
        stock.clear(); bag.clear();
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
        return it.isPotion() ? PINK : slotColor(it.slot);
    }

    Color enemyColor(const std::string& t)
    {
        if (t == "dragon")
            return DARKPURPLE;
        if (t == "skeleton")
            return LIGHTGRAY;
        return RED;
    }
    float enemyRadius(const std::string& t)
    {
        return t == "dragon" ? 0.46f : 0.35f;
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
}

void Renderer::draw(const Game& game, bool showInventory) const
{
    const Map& map = game.map();
    const Player& player = game.player();
    const int screenW = map.width() * TILE_SIZE;
    const int screenH = map.height() * TILE_SIZE;
    const bool shopOpen = game.isShopOpen();

    BeginDrawing();
    ClearBackground(DARKGRAY);

    for (int y = 0; y < map.height(); ++y)
        for (int x = 0; x < map.width(); ++x)
        {
            const int px = x * TILE_SIZE, py = y * TILE_SIZE;
            if (map.isWall(x, y))
                DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, RAYWHITE);
            else if (map.isExit(x, y))
                DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, GREEN);
        }

    for (const auto& gi : game.groundItems())
        DrawRectangle(gi.x * TILE_SIZE + TILE_SIZE / 4, gi.y * TILE_SIZE + TILE_SIZE / 4,
            TILE_SIZE / 2, TILE_SIZE / 2, itemColor(gi.item));

    for (const auto& c : game.chests())
    {
        const int px = c.x * TILE_SIZE, py = c.y * TILE_SIZE;
        DrawRectangle(px + TILE_SIZE / 6, py + TILE_SIZE / 4, TILE_SIZE * 2 / 3, TILE_SIZE / 2, BROWN);
        DrawRectangle(px + TILE_SIZE / 6, py + TILE_SIZE / 4, TILE_SIZE * 2 / 3, TILE_SIZE / 8, GOLD);
        DrawRectangleLines(px + TILE_SIZE / 6, py + TILE_SIZE / 4, TILE_SIZE * 2 / 3, TILE_SIZE / 2, BLACK);
    }

    if (game.shopkeeper().exists)
    {
        const int px = game.shopkeeper().x * TILE_SIZE, py = game.shopkeeper().y * TILE_SIZE;
        DrawCircle(px + TILE_SIZE / 2, py + TILE_SIZE / 2, TILE_SIZE * 0.38f, TEAL);
        DrawCircle(px + TILE_SIZE / 2, py + TILE_SIZE / 2, TILE_SIZE * 0.16f, GOLD);
        DrawText("$", px + TILE_SIZE / 2 - 4, py + TILE_SIZE / 2 - 8, 18, BLACK);
    }

    for (const auto& e : game.enemies())
    {
        DrawCircle(static_cast<int>(e.visualX() * TILE_SIZE) + TILE_SIZE / 2,
            static_cast<int>(e.visualY() * TILE_SIZE) + TILE_SIZE / 2,
            TILE_SIZE * enemyRadius(e.type()), enemyColor(e.type()));
        drawHpBar(static_cast<int>(e.visualX() * TILE_SIZE), static_cast<int>(e.visualY() * TILE_SIZE), e.hp(), e.maxHp());
    }
    DrawCircle(static_cast<int>(player.visualX() * TILE_SIZE) + TILE_SIZE / 2,
        static_cast<int>(player.visualY() * TILE_SIZE) + TILE_SIZE / 2, TILE_SIZE * 0.35f, YELLOW);
    drawHpBar(static_cast<int>(player.visualX() * TILE_SIZE), static_cast<int>(player.visualY() * TILE_SIZE),
        player.hp(), player.maxHp());

    DrawRectangle(0, screenH, screenW, HUD_HEIGHT, BLACK);
    DrawText(TextFormat("HP %d/%d   ATK %d   DEF %d   GOLD %d", player.hp(), player.maxHp(),
        player.attackPower(), player.defense(), game.gold()),
        10, screenH + 8, 20, player.hp() > player.maxHp() / 4 ? RAYWHITE : RED);
    DrawText("[I] Inventory", screenW - 150, screenH + 8, 18, LIGHTGRAY);
    const auto& log = game.log();
    for (int i = 0; i < 3 && i < static_cast<int>(log.size()); ++i)
        DrawText(log[log.size() - 1 - i].c_str(), 10, screenH + 34 + i * 18, 16, LIGHTGRAY);

    if (showInventory && !shopOpen)
    {
        DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.75f));
        int panelX, panelY, ringY, bagX, sy2;
        std::vector<GearBox> gear;
        std::vector<Box> bag;
        invLayout(screenW, gear, bag, panelX, panelY, ringY, bagX, sy2);
        DrawRectangle(panelX, panelY, screenW - 2 * panelX, screenH - 2 * panelY, Color{ 25,25,35,255 });
        DrawRectangleLines(panelX, panelY, screenW - 2 * panelX, screenH - 2 * panelY, RAYWHITE);
        DrawText("INVENTORY", panelX + 24, panelY + 16, 22, RAYWHITE);
        DrawText("Equipment", panelX + 24, sy2 - 24, 16, LIGHTGRAY);
        DrawText("Backpack", bagX, sy2 - 24, 16, LIGHTGRAY);

        const Inventory& inv = player.inventory();
        for (int i = 0; i < 7; ++i)
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
            drawCell(gear[7 + j].box, item);
        }
        for (int k = 0; k < 27; ++k)
        {
            const auto& slot = inv.storage()[k];
            drawCell(bag[k], slot ? &*slot : nullptr);
        }
        DrawText("Click backpack: equip gear / drink potion.  Click worn gear: unequip.  [I] close.",
            panelX + 24, screenH - panelY - 26, 16, LIGHTGRAY);
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
        const char* m = "LEVEL COMPLETE";
        DrawText(m, (screenW - MeasureText(m, 40)) / 2, screenH / 2 - 40, 40, GREEN);
        const char* s = "Press Esc to quit";
        DrawText(s, (screenW - MeasureText(s, 20)) / 2, screenH / 2 + 10, 20, RAYWHITE);
    }

    EndDrawing();
}

InvHit Renderer::hitTestInventory(const Game& game, int mx, int my) const
{
    const int screenW = game.map().width() * TILE_SIZE;
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
    const int screenW = game.map().width() * TILE_SIZE;
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

void Renderer::screenToTile(int mx, int my, int& tileX, int& tileY) const
{
    tileX = mx / TILE_SIZE;
    tileY = my / TILE_SIZE;
}