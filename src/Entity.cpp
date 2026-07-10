#include "Entity.h"
#include <algorithm>

void Entity::update(float dt)
{
    const float tx = static_cast<float>(x_), ty = static_cast<float>(y_);
    const float step = MOVE_SPEED * dt;
    if (vx_ < tx)
        vx_ = std::min(vx_ + step, tx);
    else if (vx_ > tx)
        vx_ = std::max(vx_ - step, tx);
    if (vy_ < ty)
        vy_ = std::min(vy_ + step, ty);
    else if (vy_ > ty)
        vy_ = std::max(vy_ - step, ty);
}