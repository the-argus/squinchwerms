#include "space.h"

namespace lib {
void Space::add(Body &body) noexcept { body._add_to_space(this); }
void Space::add(Shape &shape) noexcept { shape._add_to_space(this); }
void Space::remove(Body &body) noexcept { cpSpaceRemoveBody(this, &body); }
void Space::remove(Shape &shape) noexcept { cpSpaceRemoveShape(this, &shape); }
void Space::remove(cpConstraint &constraint) noexcept
{
    cpSpaceRemoveConstraint(this, &constraint);
}
void Space::remove(cpDampedSpring &constraint) noexcept
{
    remove(*(cpConstraint *)&constraint);
}
float Space::damping() const noexcept { return cpSpaceGetDamping(this); }
void Space::set_damping(float damping) noexcept
{
    cpSpaceSetDamping(this, damping);
}
Vect Space::gravity() const noexcept { return cpSpaceGetGravity(this); }
void Space::set_gravity(Vect gravity) noexcept
{
    cpSpaceSetGravity(this, gravity);
}
cpTimestamp Space::collision_persistence() const noexcept
{
    return cpSpaceGetCollisionPersistence(this);
}
void Space::set_collision_persistence(cpTimestamp persistence) noexcept
{
    cpSpaceSetCollisionPersistence(this, persistence);
}
cpDataPointer Space::user_data() const noexcept
{
    return cpSpaceGetUserData(this);
}
void Space::set_user_data(cpDataPointer data) noexcept
{
    cpSpaceSetUserData(this, data);
}
void Space::set_collision_bias(float bias) noexcept
{
    cpSpaceSetCollisionBias(this, bias);
}
float Space::collision_bias() const noexcept
{
    return cpSpaceGetCollisionBias(this);
}
void Space::set_collision_slop(float slop) noexcept
{
    cpSpaceSetCollisionSlop(this, slop);
}
float Space::collision_slop() const noexcept
{
    return cpSpaceGetCollisionSlop(this);
}
void Space::set_idle_speed_threshold(float threshold) noexcept
{
    cpSpaceSetIdleSpeedThreshold(this, threshold);
}
float Space::idle_speed_threshold() const noexcept
{
    return cpSpaceGetIdleSpeedThreshold(this);
}
int Space::iterations() const noexcept { return cpSpaceGetIterations(this); }
void Space::set_iterations(int iterations) noexcept
{
    cpSpaceSetIterations(this, iterations);
}

// read only
float Space::get_sleep_time_threshold() const noexcept
{
    return cpSpaceGetSleepTimeThreshold(this);
}
Body *Space::get_static_body() noexcept
{
    return static_cast<Body *>(cpSpaceGetStaticBody(this));
}
float Space::get_current_time_step() const noexcept
{
    return cpSpaceGetCurrentTimeStep(this);
}

void Space::step(float timestep) noexcept { cpSpaceStep(this, timestep); }
} // namespace lib
