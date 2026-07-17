#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"
#include <string>

class Enemy : public Entity
{
public:
    Enemy(std::string type, int x, int y, int hp, int attack, int sight, int defense, int roamRange);

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

    void scaleStats(float m);

    bool isBoss() const
    {
        return boss_;
    }
    void setBoss(bool b)
    {
        boss_ = b;
    }

    int  spawnX() const
    {
        return spawnX_;
    }
    int  spawnY() const
    {
        return spawnY_;
    }
    int  roamRange() const
    {
        return roamRange_;
    }
    int  aggro() const
    {
        return aggro_;
    }
    void setAggro(int a)
    {
        aggro_ = a;
    }
    int  lastSeenX() const
    {
        return lastSeenX_;
    }
    int  lastSeenY() const
    {
        return lastSeenY_;
    }
    void setLastSeen(int x, int y)
    {
        lastSeenX_ = x;
        lastSeenY_ = y;
    }

private:
    std::string type_;
    int sight_ = 0;
    bool boss_ = false;
    int spawnX_ = 0, spawnY_ = 0, roamRange_ = 0;
    int aggro_ = 0, lastSeenX_ = 0, lastSeenY_ = 0;
};

#endif