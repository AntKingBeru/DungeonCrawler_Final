#ifndef MAP_H
#define MAP_H

#include <string>
#include <vector>
#include "ConfigData.h"

enum class Tile
{
    Floor,
    Wall,
    Exit
};

class Map
{
public:
    static constexpr char WALL_CH = 'X';
    static constexpr char EXIT_CH = 'E';

    void loadFrom(const ConfigData& data);

    int width() const
    {
        return width_;
    }
    int height() const
    {
        return height_;
    }

    Tile tileAt(int x, int y) const;

    bool isWall(int x, int y) const
    {
		return tileAt(x, y) == Tile::Wall;
    }

    bool isExit(int x, int y) const
    {
        return tileAt(x, y) == Tile::Exit;
    }

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<std::string> grid_;
};

#endif