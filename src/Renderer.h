#ifndef RENDERER_H
#define RENDERER_H

class Game;

constexpr int TILE_SIZE = 40;
constexpr int HUD_HEIGHT = 90;

class Renderer
{
public:
    void draw(const Game& game) const;
};

#endif