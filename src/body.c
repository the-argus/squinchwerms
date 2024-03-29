#include "body.h"
#include "chipmunk/chipmunk_structs.h"
#include <cassert>
#include <cstring>

namespace lib {
float Body::torque() const { return cpBodyGetTorque(this); }
void Body::set_torque(float torque) { return cpBodySetTorque(this, torque); }
float Body::moment() const { return cpBodyGetMoment(this); }
void Body::set_moment(float moment) { cpBodySetMoment(this, moment); }
Vect Body::velocity() const { return cpBodyGetVelocity(this); }
void Body::set_velocity(Vect velocity) { cpBodySetVelocity(this, velocity); }
Vect Body::position() const { return cpBodyGetPosition(this); }
void Body::set_position(Vect position) { cpBodySetPosition(this, position); }
Vect Body::force() const { return cpBodyGetForce(this); }
void Body::set_force(Vect force) { cpBodySetForce(this, force); }
float Body::angle() const { return cpBodyGetAngle(this); }
void Body::set_angle(float angle) { cpBodySetAngle(this, angle); }

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
    remove_from_space();
    // this function does nothing as of cp 7.0.3
    cpBodyDestroy(this);
}
void Body::remove_from_space()
{
    if (space != nullptr)
        cpSpaceRemoveBody(space, this);
}

cpDampedSpring *
Body::connect_with_damped_spring(cpDampedSpring *connection, Body *other,
                                 Vect point_on_this, Vect point_on_other,
                                 const spring_options_t &options)
{
    assert(space != nullptr);

    auto constraint = (cpConstraint *)cpDampedSpringInit(
        connection, this, other, point_on_this, point_on_other, options.length,
        options.stiffness, options.damping);

    cpSpaceAddConstraint(space, constraint);

    return connection;
}

cpDampedSpring *
Body::connect_with_damped_spring(cpDampedSpring *connection, Body *other,
                                 const spring_options_t &options)
{
    return connect_with_damped_spring(connection, other, {0, 0}, {0, 0},
                                      options);
}

cpSimpleMotor *
Body::connect_with_simple_motor(cpSimpleMotor *motor, Body *other,
                                const simple_motor_options_t &options)
{
    assert(space != nullptr);

    cpSimpleMotorInit(motor, this, other, options.rate);

    cpSpaceAddConstraint(space, &motor->constraint);

    return motor;
}
} // namespace lib
