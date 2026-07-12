#include "Menu.h"
#include "raylib.h"

namespace
{
    constexpr int BTN_W = 460, BTN_H = 54, BTN_GAP = 16, COUNT = 5;

    struct R
    {
        int x, y, w, h;
        bool has(int px, int py) const
        {
            return px >= x && px < x + w && py >= y && py < y + h;
        }
    };

    R buttonRect(int screenW, int screenH, int i)
    {
        const int totalH = COUNT * BTN_H + (COUNT - 1) * BTN_GAP;
        const int startY = screenH / 2 - totalH / 2 + 40;
        return { screenW / 2 - BTN_W / 2, startY + i * (BTN_H + BTN_GAP), BTN_W, BTN_H };
    }

    void drawButton(const R& r, const std::string& label, bool enabled)
    {
        const Vector2 m = GetMousePosition();
        const bool hover = enabled && r.has(static_cast<int>(m.x), static_cast<int>(m.y));
        DrawRectangle(r.x, r.y, r.w, r.h, hover ? Color{ 55,70,90,255 } : Color{ 35,40,55,255 });
        DrawRectangleLines(r.x, r.y, r.w, r.h, enabled ? RAYWHITE : Color{ 80,80,90,255 });
        DrawText(label.c_str(), r.x + 20, r.y + r.h / 2 - 10, 20, enabled ? RAYWHITE : GRAY);
    }

    void drawScreen(int w, int h, const char* title, const char* const labels[COUNT],
        const bool enabled[COUNT])
    {
        DrawText(title, w / 2 - MeasureText(title, 44) / 2, h / 2 - (COUNT * (BTN_H + BTN_GAP)) / 2 - 40, 44, RAYWHITE);
        for (int i = 0; i < COUNT; ++i)
            drawButton(buttonRect(w, h, i), labels[i], enabled[i]);
    }

    int hit(int w, int h, int mx, int my, const bool enabled[COUNT])
    {
        for (int i = 0; i < COUNT; ++i)
            if (enabled[i] && buttonRect(w, h, i).has(mx, my))
                return i;
        return -1;
    }
}

void Menu::drawMain(int w, int h, const SlotInfo& slots) const
{
    const std::string l1 = "Load Slot 1:  " + slots[0];
    const std::string l2 = "Load Slot 2:  " + slots[1];
    const std::string l3 = "Load Slot 3:  " + slots[2];
    const char* labels[COUNT] = { "New Game", l1.c_str(), l2.c_str(), l3.c_str(), "Quit" };

    const bool enabled[COUNT] = { true, slots[0] != "Empty", slots[1] != "Empty", slots[2] != "Empty", true };
    drawScreen(w, h, "DUNGEON CRAWLER", labels, enabled);
}

int Menu::hitMain(int w, int h, int mx, int my) const
{
    const bool enabled[COUNT] = { true, true, true, true, true };
    return hit(w, h, mx, my, enabled);
}

void Menu::drawPause(int w, int h, const SlotInfo& slots) const
{
    DrawRectangle(0, 0, w, h, Fade(BLACK, 0.7f));
    const std::string l1 = "Save to Slot 1:  " + slots[0];
    const std::string l2 = "Save to Slot 2:  " + slots[1];
    const std::string l3 = "Save to Slot 3:  " + slots[2];
    const char* labels[COUNT] = { "Resume", l1.c_str(), l2.c_str(), l3.c_str(), "Main Menu" };
    const bool enabled[COUNT] = { true, true, true, true, true };
    drawScreen(w, h, "PAUSED", labels, enabled);
}

int Menu::hitPause(int w, int h, int mx, int my) const
{
    const bool enabled[COUNT] = { true, true, true, true, true };
    return hit(w, h, mx, my, enabled);
}