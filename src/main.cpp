// Entry point: main menu + pause/save menu wrapped around the play loop. Thin composition root.
#include "Game.h"
#include "Renderer.h"
#include "Menu.h"
#include "ParserFactory.h"
#include "raylib.h"
#include <array>
#include <string>
#include <filesystem>

namespace
{
    constexpr int kMenuW = 900, kMenuH = 640;
    const std::string kLevel = "configs/dungeon.ini";
    const std::array<std::string, 3> kSlots = {
        "saves/slot1.json", "saves/slot2.json", "saves/slot3.json" };

    std::string slotSummary(const std::string& path)
    {
        if (!std::filesystem::exists(path))
            return "Empty";
        try
        {
            ConfigData d = makeParser(path)->parse(path);
            const std::string name = (d.count("save") && d["save"].count("name")) ? d["save"]["name"] : "Saved";
            const std::string gold = (d.count("player") && d["player"].count("gold")) ? d["player"]["gold"] : "0";
            return name + " - " + gold + "g";
        }
        catch (...)
        {
            return "Empty";
        }
    }
    SlotInfo readSlots()
    {
        return { slotSummary(kSlots[0]), slotSummary(kSlots[1]), slotSummary(kSlots[2]) };
    }
}

int main()
{
    InitWindow(kMenuW, kMenuH, "Dungeon Crawler");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    Game game;
    Renderer renderer;
    Menu menu;

    enum class State
    {
        Menu,
        Play
    };
    State state = State::Menu;
    bool inventoryOpen = false, paused = false, quit = false;
    SlotInfo slots = readSlots();

    auto sizeToGame = [&]
        {
            SetWindowSize(game.map().width() * TILE_SIZE, game.map().height() * TILE_SIZE + HUD_HEIGHT);
        };
    auto toMenu = [&]
        {
            state = State::Menu;
            inventoryOpen = paused = false;
            game.closeShop();
            slots = readSlots();
            SetWindowSize(kMenuW, kMenuH);
        };

    while (!quit && !WindowShouldClose())
    {
        const int W = GetScreenWidth(), H = GetScreenHeight();
        if (state == State::Play)
            game.update(GetFrameTime());

        if (state == State::Menu)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                const int a = menu.hitMain(W, H, GetMouseX(), GetMouseY());
                if (a == 0)
                {
                    game.newGame(kLevel);
                    sizeToGame();
                    state = State::Play;
                    inventoryOpen = paused = false;
                }
                else if (a >= 1 && a <= 3)
                {
                    if (slots[a - 1] != "Empty" && game.loadSlot(kSlots[a - 1]))
                    {
                        sizeToGame();
                        state = State::Play;
                        inventoryOpen = paused = false;
                    }
                }
                else if (a == 4)
                    quit = true;
            }
        }
        else
        {
            const bool ended = game.isComplete() || game.isGameOver();

            if (IsKeyPressed(KEY_ESCAPE))
            {
                if (ended)
                    toMenu();
                else if (game.isShopOpen())
                    game.closeShop();
                else if (inventoryOpen)
                    inventoryOpen = false;
                else if (paused)
                    paused = false;
                else
                {
                    paused = true;
                    slots = readSlots();
                }
            }

            if (state == State::Play && !ended && !paused)
            {
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
            }
            else if (state == State::Play && paused)
            {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    const int a = menu.hitPause(W, H, GetMouseX(), GetMouseY());
                    if (a == 0)
                        paused = false;
                    else if (a >= 1 && a <= 3)
                    {
                        std::filesystem::create_directories("saves");
                        game.saveSlot(kSlots[a - 1]);
                        slots = readSlots();
                        paused = false;
                    }
                    else if (a == 4)
                        toMenu();
                }
            }
        }

        BeginDrawing();
        if (state == State::Menu)
        {
            ClearBackground(Color{ 15,15,22,255 });
            menu.drawMain(GetScreenWidth(), GetScreenHeight(), slots);
        }
        else
        {
            ClearBackground(DARKGRAY);
            renderer.draw(game, inventoryOpen);
            if (paused)
                menu.drawPause(GetScreenWidth(), GetScreenHeight(), slots);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}