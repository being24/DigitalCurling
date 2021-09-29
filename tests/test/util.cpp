#include "util.hpp"

namespace digital_curling::simulation {

bool operator == (StoneData const& a, StoneData const& b)
{
    return
        a.position.x == b.position.x &&
        a.position.y == b.position.y &&
        a.angle == b.angle &&
        a.linear_velocity.x == b.linear_velocity.x &&
        a.linear_velocity.y == b.linear_velocity.y &&
        a.angular_velocity == b.angular_velocity;
}

bool operator == (StoneCollision const& a, StoneCollision const& b)
{
    return
        a.a_id == b.a_id &&
        a.b_id == b.b_id &&
        a.a_position.x == b.a_position.x &&
        a.a_position.y == b.a_position.y &&
        a.b_position.x == b.b_position.x &&
        a.b_position.y == b.b_position.y &&
        a.normal_impulse == b.normal_impulse &&
        a.tangent_impulse == b.tangent_impulse;
}

} // namespace digital_curling::simulation
