#ifndef ENTITY_H
#define ENTITY_H

class Entity
{
public:
	virtual ~Entity() = default;

	void setTile(int x, int y)
	{
		x_ = x;
		y_ = y;
	}
	void snapVisual()
	{
		vx_ = static_cast<float>(x_);
		vy_ = static_cast<float>(y_);
	}
	void update(float dt);

	int x() const
	{
		return x_;
	}
	int y() const
	{
		return y_;
	}
	float visualX() const
	{
		return vx_;
	}
	float visualY() const
	{
		return vy_;
	}
	bool isMoving() const
	{
		return vx_ != static_cast<float>(x_) || vy_ != static_cast<float>(y_);
	}

	int hp() const
	{
		return hp_;
	}
	int maxHp() const
	{
		return maxHp_;
	}
	int attackPower() const
	{
		return attack_;
	}
	int defense() const
	{
		return defense_;
	}
	bool alive() const
	{
		return hp_ > 0;
	}
	void takeDamage(int d)
	{
		hp_ -= d;
		if (hp_ < 0)
			hp_ = 0;
	}
	void heal(int d)
	{
		hp_ += d;
		if (hp_ > maxHp_)
			hp_ = maxHp_;
	}

protected:
	static constexpr float MOVE_SPEED = 8.0f;
	int x_ = 0, y_ = 0;
	float vx_ = 0, vy_ = 0;
	int hp_ = 1, maxHp_ = 1, attack_ = 0, defense_ = 0;
};

#endif;