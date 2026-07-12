#ifndef MENU_H
#define MENU_H

#include <string>
#include <array>

using SlotInfo = std::array<std::string, 3>;

class Menu
{
public:
    void drawMain(int screenW, int screenH, const SlotInfo& slots) const;
    void drawPause(int screenW, int screenH, const SlotInfo& slots) const;

    int hitMain(int screenW, int screenH, int mx, int my) const;
    int hitPause(int screenW, int screenH, int mx, int my) const;
};

#endif