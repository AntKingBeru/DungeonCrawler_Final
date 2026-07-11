#ifndef RENDERER_H
#define RENDERER_H

#include "Item.h"

class Game;

constexpr int TILE_SIZE = 40;
constexpr int HUD_HEIGHT = 90;

struct InvHit
{
    enum Kind
    {
        None,
        Storage,
        Gear
    };
    Kind kind = None;
    int storageIndex = -1;
    ItemSlot slot = ItemSlot::Weapon;
    int sub = -1;
};

struct ShopHit
{
    enum Kind
    {
        None,
        Buy,
        Sell,
        Close
    };
    Kind kind = None;
    int index = -1;
};

class Renderer
{
public:
    void draw(const Game& game, bool showInventory) const;

    InvHit hitTestInventory(const Game& game, int mx, int my) const;
    ShopHit hitTestShop(const Game& game, int mx, int my) const;

    void screenToTile(int mx, int my, int& tileX, int& tileY) const;
};

#endif