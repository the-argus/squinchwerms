#include "space.h"

namespace lib {
void Space::add(Body &body) noexcept { body._add_to_space(this); }
void Space::add(Shape &shape) noexcept { shape._add_to_space(*this); }
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
Space &Space::setDamping(float damping) noexcept
{
    cpSpaceSetDamping(this, damping);
    return *this;
}
Vect Space::gravity() const noexcept { return cpSpaceGetGravity(this); }
Space &Space::setGravity(Vect gravity) noexcept
{
    cpSpaceSetGravity(this, gravity);
    return *this;
}
cpTimestamp Space::collisionPersistence() const noexcept
{
    return cpSpaceGetCollisionPersistence(this);
}
Space &Space::setCollisionPersistence(cpTimestamp persistence) noexcept
{
    cpSpaceSetCollisionPersistence(this, persistence);
    return *this;
}
cpDataPointer Space::userData() const noexcept
{
    return cpSpaceGetUserData(this);
}
Space &Space::setUserData(cpDataPointer data) noexcept
{
    cpSpaceSetUserData(this, data);
    return *this;
}
Space &Space::setCollisionBias(float bias) noexcept
{
    cpSpaceSetCollisionBias(this, bias);
    return *this;
}
float Space::collisionBias() const noexcept
{
    return cpSpaceGetCollisionBias(this);
}
Space &Space::setCollisionSlop(float slop) noexcept
{
    cpSpaceSetCollisionSlop(this, slop);
    return *this;
}
float Space::collisionSlop() const noexcept
{
    return cpSpaceGetCollisionSlop(this);
}
Space &Space::setIdleSpeedThreshold(float threshold) noexcept
{
    cpSpaceSetIdleSpeedThreshold(this, threshold);
    return *this;
}
float Space::idleSpeedThreshold() const noexcept
{
    return cpSpaceGetIdleSpeedThreshold(this);
}
int Space::iterations() const noexcept { return cpSpaceGetIterations(this); }
Space &Space::setIterations(int iterations) noexcept
{
    cpSpaceSetIterations(this, iterations);
    return *this;
}

// read only
float Space::getSleepTimeThreshold() const noexcept
{
    return cpSpaceGetSleepTimeThreshold(this);
}
Body &Space::getStaticBody() noexcept
{
    return *static_cast<Body *>(cpSpaceGetStaticBody(this));
}
float Space::getCurrentTimeStep() const noexcept
{
    return cpSpaceGetCurrentTimeStep(this);
}

void Space::step(float timestep) noexcept { cpSpaceStep(this, timestep); }
} // namespace lib
