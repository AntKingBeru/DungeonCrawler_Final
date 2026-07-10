#include "Map.h"
#include "ConfigUtil.h"

void Map::loadFrom(const ConfigData& data)
{
    width_ = cfg::requireInt(data, "map", "width");
    height_ = cfg::requireInt(data, "map", "height");

    grid_.clear();
    grid_.reserve(height_);
    for (int y = 0; y < height_; ++y)
        grid_.push_back(cfg::require(data, "map", "row" + std::to_string(y)));
}

Tile Map::tileAt(int x, int y) const
{
    if (x < 0 || y < 0 || x >= width_ || y >= height_)
        return Tile::Wall;
    if (y >= static_cast<int>(grid_.size()))
        return Tile::Wall;
    const std::string& row = grid_[y];
    if (x >= static_cast<int>(row.size()))
        return Tile::Wall;
    const char c = row[x];
    if (c == WALL_CH)
        return Tile::Wall;
    if (c == EXIT_CH)
        return Tile::Exit;
    return Tile::Floor;
}