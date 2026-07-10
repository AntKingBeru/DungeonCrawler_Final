#include "Renderer.h"
#include "Game.h"
#include "raylib.h"
#include <string>

static void drawHpBar(float vx, float vy, int hp, int maxHp)
{
    if (maxHp <= 0)
        return;
    const int w = static_cast<int>(TILE_SIZE * 0.7f), h = 4;
    const int x = static_cast<int>(vx * TILE_SIZE) + (TILE_SIZE - w) / 2;
    const int y = static_cast<int>(vy * TILE_SIZE) + 2;
    DrawRectangle(x, y, w, h, MAROON);
    DrawRectangle(x, y, static_cast<int>(w * (static_cast<float>(hp) / maxHp)), h, LIME);
}

void Renderer::draw(const Game& game) const
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
            else if (map.isExit(x, y))
                DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, GREEN);
        }

    for (const auto& e : game.enemies())
    {
        const int cx = static_cast<int>(e.visualX() * TILE_SIZE) + TILE_SIZE / 2;
        const int cy = static_cast<int>(e.visualY() * TILE_SIZE) + TILE_SIZE / 2;
        DrawCircle(cx, cy, TILE_SIZE * 0.35f, RED);
        drawHpBar(e.visualX(), e.visualY(), e.hp(), e.maxHp());
    }

    const int cx = player.x() * TILE_SIZE + TILE_SIZE / 2;
    const int cy = player.y() * TILE_SIZE + TILE_SIZE / 2;
    DrawCircle(cx, cy, TILE_SIZE * 0.35f, YELLOW);
    drawHpBar(player.visualX(), player.visualY(), player.hp(), player.maxHp());

    DrawRectangle(0, mapPixH, mapPixW, HUD_HEIGHT, BLACK);
    DrawText(TextFormat("HP: %d / %d", player.hp(), player.maxHp()),
        10, mapPixH + 8, 20, player.hp() > player.maxHp() / 4 ? RAYWHITE : RED);
    const auto& log = game.log();
    for (int i = 0; i < 3 && i < static_cast<int>(log.size()); ++i)
        DrawText(log[log.size() - 1 - i].c_str(), 10, mapPixH + 34 + i * 18, 16, LIGHTGRAY);

    if (game.isGameOver())
    {
        DrawRectangle(0, 0, mapPixW, mapPixH, Fade(BLACK, 0.6f));
        const char* m = "YOU DIED";
        DrawText(m, (mapPixW - MeasureText(m, 48)) / 2, mapPixH / 2 - 24, 48, RED);
    }
    else if (game.isComplete() && !player.isMoving())
    {
        DrawRectangle(0, 0, mapPixW, mapPixH, Fade(BLACK, 0.6f));
        const char* m = "LEVEL COMPLETE";
        DrawText(m, (mapPixW - MeasureText(m, 40)) / 2, mapPixH / 2 - 40, 40, GREEN);
        const char* s = "Press Esc to quit";
        DrawText(s, (mapPixW - MeasureText(s, 20)) / 2, mapPixH / 2 + 10, 20, RAYWHITE);
    }

    EndDrawing();
}