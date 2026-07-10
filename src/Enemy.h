#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"
#include <string>

class Enemy : public Entity
{
public:
	Enemy(std::string type, int x, int y, int hp, int attack, int sight, std::string reward);

	static Enemy makeGoblin(int x, int y);

	const std::string& type() const
	{
		return type_;
	}
	const std::string& reward() const
	{
		return reward_;
	}
	int sight() const
	{
		return sight_;
	}

private:
	std::string type_;
	std::string reward_;
	int sight_ = 0;
};

#endif