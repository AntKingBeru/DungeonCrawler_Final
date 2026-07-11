#include "Renderer.h"
#include "Game.h"
#include "raylib.h"
#include <string>
#include <vector>

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
    { Box box;
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

    void layout(int winW, int mapPixH, std::vector<GearBox>& gear, std::vector<Box>& bag,
        int& panelX, int& panelY, int& ringY, int& bagX, int& sy)
    {
        panelX = 40; panelY = 40;
        const int sx = panelX + 24;
        sy = panelY + 64;
        gear.clear(); bag.clear();
        for (int i = 0; i < 7; ++i)
            gear.push_back({ { sx + LABELW, sy + i * (BOX + GAP), BOX, BOX }, kSingles[i].slot, 0 });
        ringY = sy + 7 * (BOX + GAP) + 26;
        for (int j = 0; j < 8; ++j)
            gear.push_back({ { sx + j * (BOX + GAP), ringY, BOX, BOX }, ItemSlot::Ring, j });
        bagX = winW / 2 + 20;
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

    void drawHpBar(float vx, float vy, int hp, int maxHp)
    {
        if (maxHp <= 0)
            return;
        const int w = static_cast<int>(TILE_SIZE * 0.7f), h = 4;
        const int x = static_cast<int>(vx * TILE_SIZE) + (TILE_SIZE - w) / 2;
        const int y = static_cast<int>(vy * TILE_SIZE) + 2;
        DrawRectangle(x, y, w, h, MAROON);
        DrawRectangle(x, y, static_cast<int>(w * (static_cast<float>(hp) / maxHp)), h, LIME);
    }

    void drawCell(const Box& b, const Item* item)
    {
        if (item)
        {
            DrawRectangle(b.x, b.y, b.w, b.h, slotColor(item->slot));
            char c[2] = { item->name.empty() ? '?' : item->name[0], 0 };
            DrawText(c, b.x + b.w / 2 - 5, b.y + b.h / 2 - 8, 18, BLACK);
        }
        else
        {
            DrawRectangle(b.x, b.y, b.w, b.h, Color{ 40,40,50,255 });
        }
        DrawRectangleLines(b.x, b.y, b.w, b.h, GRAY);
    }
}

void Renderer::draw(const Game& game, bool showInventory) const
{
    const Map& map = game.map();
    const Player& player = game.player();
    const int mapPixW = map.width() * TILE_SIZE;
    const int mapPixH = map.height() * TILE_SIZE;

    BeginDrawing();
    ClearBackground(DARKGRAY);

    for (int y = 0; y < map.height(); ++y)
        for (int x = 0; x < map.width(); ++x)
        {
            const int px = x * TILE_SIZE, py = y * TILE_SIZE;
            if (map.isWall(x, y))
                DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, RAYWHITE);
            else if
                (map.isExit(x, y)) DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, GREEN);
        }

    for (const auto& gi : game.groundItems())
        DrawRectangle(gi.x * TILE_SIZE + TILE_SIZE / 4, gi.y * TILE_SIZE + TILE_SIZE / 4,
            TILE_SIZE / 2, TILE_SIZE / 2, slotColor(gi.item.slot));

    for (const auto& e : game.enemies())
    {
        DrawCircle(static_cast<int>(e.visualX() * TILE_SIZE) + TILE_SIZE / 2,
            static_cast<int>(e.visualY() * TILE_SIZE) + TILE_SIZE / 2,
            TILE_SIZE * enemyRadius(e.type()), enemyColor(e.type()));
        drawHpBar(e.visualX(), e.visualY(), e.hp(), e.maxHp());
    }
    DrawCircle(static_cast<int>(player.visualX() * TILE_SIZE) + TILE_SIZE / 2,
        static_cast<int>(player.visualY() * TILE_SIZE) + TILE_SIZE / 2, TILE_SIZE * 0.35f, YELLOW);
    drawHpBar(player.visualX(), player.visualY(), player.hp(), player.maxHp());

    DrawRectangle(0, mapPixH, mapPixW, HUD_HEIGHT, BLACK);
    DrawText(TextFormat("HP %d/%d   ATK %d   DEF %d", player.hp(), player.maxHp(),
        player.attackPower(), player.defense()),
        10, mapPixH + 8, 20, player.hp() > player.maxHp() / 4 ? RAYWHITE : RED);
    DrawText("[I] Inventory", mapPixW - 150, mapPixH + 8, 18, LIGHTGRAY);
    const auto& log = game.log();
    for (int i = 0; i < 3 && i < static_cast<int>(log.size()); ++i)
        DrawText(log[log.size() - 1 - i].c_str(), 10, mapPixH + 34 + i * 18, 16, LIGHTGRAY);

    if (showInventory)
    {
        DrawRectangle(0, 0, mapPixW, mapPixH, Fade(BLACK, 0.75f));
        int panelX, panelY, ringY, bagX, sy; std::vector<GearBox> gear; std::vector<Box> bag;
        layout(mapPixW, mapPixH, gear, bag, panelX, panelY, ringY, bagX, sy);
        DrawRectangle(panelX, panelY, mapPixW - 2 * panelX, mapPixH - 2 * panelY, Color{ 25,25,35,255 });
        DrawRectangleLines(panelX, panelY, mapPixW - 2 * panelX, mapPixH - 2 * panelY, RAYWHITE);
        DrawText("INVENTORY", panelX + 24, panelY + 20, 24, RAYWHITE);
        DrawText("Equipment", panelX + 24, sy - 26, 18, LIGHTGRAY);
        DrawText("Backpack", bagX, sy - 26, 18, LIGHTGRAY);

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
            const Item* item = (k < static_cast<int>(inv.storage().size())) ? &inv.storage()[k] : nullptr;
            drawCell(bag[k], item);
        }
        DrawText("Click a backpack item to equip; click worn gear to unequip.  [I] to close.",
            panelX + 24, mapPixH - panelY - 28, 16, LIGHTGRAY);
    }

    if (!showInventory && game.isGameOver())
    {
        DrawRectangle(0, 0, mapPixW, mapPixH, Fade(BLACK, 0.6f));
        const char* m = "YOU DIED";
        DrawText(m, (mapPixW - MeasureText(m, 48)) / 2, mapPixH / 2 - 24, 48, RED);
    }
    else if (!showInventory && game.isComplete() && !player.isMoving())
    {
        DrawRectangle(0, 0, mapPixW, mapPixH, Fade(BLACK, 0.6f));
        const char* m = "LEVEL COMPLETE";
        DrawText(m, (mapPixW - MeasureText(m, 40)) / 2, mapPixH / 2 - 40, 40, GREEN);
        const char* s = "Press Esc to quit";
        DrawText(s, (mapPixW - MeasureText(s, 20)) / 2, mapPixH / 2 + 10, 20, RAYWHITE);
    }

    EndDrawing();
}

InvHit Renderer::hitTestInventory(const Game& game, int mx, int my) const
{
    const int mapPixW = game.map().width() * TILE_SIZE;
    const int mapPixH = game.map().height() * TILE_SIZE;
    int panelX, panelY, ringY, bagX, sy; std::vector<GearBox> gear; std::vector<Box> bag;
    layout(mapPixW, mapPixH, gear, bag, panelX, panelY, ringY, bagX, sy);

    const Inventory& inv = game.player().inventory();

    for (const auto& g : gear)
        if (g.box.has(mx, my) && g.sub < inv.equippedCount(g.slot))
            return InvHit{ InvHit::Gear, -1, g.slot, g.sub };

    for (int k = 0; k < 27; ++k)
        if (bag[k].has(mx, my) && k < static_cast<int>(inv.storage().size()))
            return InvHit{ InvHit::Storage, k, ItemSlot::Weapon, -1 };
    return InvHit{};
}