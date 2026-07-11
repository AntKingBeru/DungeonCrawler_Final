#include "Game.h"
#include "Renderer.h"
#include "raylib.h"
#include <iostream>

int main(int argc, char* argv[])
{
    const std::string configPath = (argc >= 2) ? argv[1] : "configs/dungeon.ini";

    Game game;
    try
    {
        game.load(configPath);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to load '" << configPath << "': " << e.what() << '\n';
        return 1;
    }

    InitWindow(game.map().width() * TILE_SIZE,
        game.map().height() * TILE_SIZE + HUD_HEIGHT, "Dungeon Crawler");
    SetTargetFPS(60);

    Renderer renderer;
    bool inventoryOpen = false;

    while (!WindowShouldClose())
    {
        game.update(GetFrameTime());

        if (game.isShopOpen())
        {
            if (IsKeyPressed(KEY_I))
                game.closeShop();
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                ShopHit h = renderer.hitTestShop(game, GetMouseX(), GetMouseY());
                if (h.kind == ShopHit::Buy)
                    game.buy(h.index);
                else if (h.kind == ShopHit::Sell)
                    game.sellBackpack(h.index);
                else if (h.kind == ShopHit::Close)
                    game.closeShop();
            }
        }
        else if (inventoryOpen)
        {
            if (IsKeyPressed(KEY_I)) inventoryOpen = false;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                InvHit hit = renderer.hitTestInventory(game, GetMouseX(), GetMouseY());
                if (hit.kind == InvHit::Storage)
                    game.useBackpackItem(hit.storageIndex);
                else if (hit.kind == InvHit::Gear)
                    game.unequipItem(hit.slot, hit.sub);
            }
        }
        else
        {
            if (IsKeyPressed(KEY_I))
                inventoryOpen = true;
            if (IsKeyDown(KEY_W))
                game.movePlayer(0, -1);
            else if (IsKeyDown(KEY_S))
                game.movePlayer(0, 1);
            else if (IsKeyDown(KEY_A))
                game.movePlayer(-1, 0);
            else if (IsKeyDown(KEY_D))
                game.movePlayer(1, 0);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                int tx, ty;
                renderer.screenToTile(GetMouseX(), GetMouseY(), tx, ty);
                game.interactAt(tx, ty);
            }
        }

        renderer.draw(game, inventoryOpen);
    }

    CloseWindow();
    return 0;
}