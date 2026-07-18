#ifndef RENDERER_H
#define RENDERER_H

#include "Item.h"

class Game;

constexpr int SCREEN_W = 1280;
constexpr int SCREEN_H = 720;
constexpr int TILE_SIZE = 30;
constexpr int HUD_HEIGHT = 90;
constexpr int VIEW_H = SCREEN_H - HUD_HEIGHT;

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

    void screenToTile(const Game& game, int mx, int my, int& tileX, int& tileY) const;
};

#endif