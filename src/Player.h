#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "ConfigData.h"

class Player : public Entity
{
public:
    void loadFrom(const ConfigData& data);
};

#endif