#include "body.h"
#include <cassert>
#include <cstring>

namespace lib {
float Body::torque() const { return cpBodyGetTorque(this); }
Body &Body::setTorque(float torque)
{
    cpBodySetTorque(this, torque);
    return *this;
}
float Body::moment() const { return cpBodyGetMoment(this); }
Body &Body::setMoment(float moment)
{
    cpBodySetMoment(this, moment);
    return *this;
}
Vect Body::velocity() const { return cpBodyGetVelocity(this); }
Body &Body::setVelocity(Vect velocity)
{
    cpBodySetVelocity(this, velocity);
    return *this;
}
Vect Body::position() const { return cpBodyGetPosition(this); }
Body &Body::setPosition(Vect position)
{
    cpBodySetPosition(this, position);
    return *this;
}
Vect Body::force() const { return cpBodyGetForce(this); }
Body &Body::setForce(Vect force)
{
    cpBodySetForce(this, force);
    return *this;
}
float Body::angle() const { return cpBodyGetAngle(this); }
Body &Body::setAngle(float angle)
{
    cpBodySetAngle(this, angle);
    return *this;
}

// read-only
lib::Body::Type Body::type() { return lib::Body::Type(cpBodyGetType(this)); }

void Body::_add_to_space(Space *_space)
{
    // HACK: this shouldnt be reinterpret_cast
    cpSpaceAddBody(reinterpret_cast<cpSpace *>(_space), this);
    space = reinterpret_cast<cpSpace *>(_space);
}

void Body::free()
{
    removeFromSpace();
    // this function does nothing as of cp 7.0.3
    cpBodyDestroy(this);
}
void Body::removeFromSpace()
{
    if (space != nullptr)
        cpSpaceRemoveBody(space, this);
}

cpDampedSpring *
Body::connectWithDampedSpringAtPoints(cpDampedSpring *connection, Body *other,
                                      Vect point_on_this, Vect point_on_other,
                                      const SpringOptions &options)
{
    assert(space != nullptr);

    auto constraint = (cpConstraint *)cpDampedSpringInit(
        connection, this, other, point_on_this, point_on_other, options.length,
        options.stiffness, options.damping);

    cpSpaceAddConstraint(space, constraint);

    return connection;
}

cpDampedSpring *Body::connectWithDampedSpring(cpDampedSpring *connection,
                                              Body *other,
                                              const SpringOptions &options)
{
    return connectWithDampedSpringAtPoints(connection, other, {0, 0}, {0, 0},
                                           options);
}

cpSimpleMotor *Body::connectWithSimpleMotor(cpSimpleMotor *motor, Body *other,
                                            const SimpleMotorOptions &options)
{
    assert(space != nullptr);

    cpSimpleMotorInit(motor, this, other, options.rate);

    cpSpaceAddConstraint(space, &motor->constraint);

    return motor;
}
} // namespace lib
