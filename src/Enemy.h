#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"
#include <string>

class Enemy : public Entity
{
public:
    Enemy(std::string type, int x, int y, int hp, int attack, int sight, int defense);

    static Enemy makeGoblin(int x, int y);
    static Enemy makeSkeleton(int x, int y);
    static Enemy makeDragon(int x, int y);

    const std::string& type() const
    {
        return type_;
    }
    int sight() const
    {
        return sight_;
    }

private:
	std::string type_;
	int sight_ = 0;
};

#endif