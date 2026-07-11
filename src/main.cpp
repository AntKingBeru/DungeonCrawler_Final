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

        if (IsKeyPressed(KEY_I))
            inventoryOpen = !inventoryOpen;

        if (inventoryOpen)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                InvHit hit = renderer.hitTestInventory(game, GetMouseX(), GetMouseY());
                if (hit.kind == InvHit::Storage)
                    game.equipItem(hit.storageIndex);
                else if (hit.kind == InvHit::Gear)
                    game.unequipItem(hit.slot, hit.sub);
            }
        }
        else
        {
            if (IsKeyDown(KEY_W))
                game.movePlayer(0, -1);
            else if (IsKeyDown(KEY_S))
                game.movePlayer(0, 1);
            else if (IsKeyDown(KEY_A))
                game.movePlayer(-1, 0);
            else if (IsKeyDown(KEY_D))
                game.movePlayer(1, 0);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                game.attackAt(GetMouseX() / TILE_SIZE, GetMouseY() / TILE_SIZE);
        }

        renderer.draw(game, inventoryOpen);
    }

    CloseWindow();
    return 0;
}