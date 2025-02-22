#pragma once
#include "shape.h"
#include "vect.h"
#include <cstring>

namespace lib {
class Body : public ::cpBody
{
  public:
    enum class Type : uint8_t
    {
        DYNAMIC = ::CP_BODY_TYPE_DYNAMIC,
        KINEMATIC = ::CP_BODY_TYPE_KINEMATIC,
        STATIC = ::CP_BODY_TYPE_STATIC,
    };
    Body() = delete;

    struct BodyOptions
    {
        Type type;
        float mass;
        float moment;
    };

    constexpr Body(const BodyOptions &options)
    {
        std::memset(static_cast<cpBody *>(this), 0, sizeof(cpBody));
        /// Initialize fields used by chipmunk to determine what type something
        /// is we have to do this here instead of in cpBodySetType because
        /// cpBodySetType reads from the values before writing to them, spamming
        /// valgrind with uninitialized memory errors
        /// WARNING: this is directly reliant on the code in cpBodyGetType and
        /// may break if chipmunk updates
        switch (options.type) {
        case Type::STATIC:
            sleeping.idleTime = INFINITY;
            break;
        case Type::DYNAMIC:
            break;
        case Type::KINEMATIC:
            m = INFINITY;
            break;
        }
        cpBodySetType(this, cpBodyType(options.type));
        cpBodyInit(this, options.mass, options.moment);
    }

    // fields
    [[nodiscard]] float torque() const;
    Body &setTorque(float torque);
    [[nodiscard]] float moment() const;
    Body &setMoment(float moment);
    [[nodiscard]] Vect velocity() const;
    Body &setVelocity(Vect velocity);
    [[nodiscard]] Vect position() const;
    Body &setPosition(Vect position);
    [[nodiscard]] Vect force() const;
    Body &setForce(Vect force);
    [[nodiscard]] float angle() const;
    Body &setAngle(float angle);
    [[nodiscard]] cpDataPointer userData() const;
    Body &setUserData(cpDataPointer data);

    void free();

    // read-only
    [[nodiscard]] Type type();

    struct SpringOptions
    {
        float length;
        float stiffness;
        float damping;
    };

    cpDampedSpring *connectWithDampedSpring(cpDampedSpring *connection,
                                            Body *other,
                                            const SpringOptions &options);
    cpDampedSpring *
    connectWithDampedSpringAtPoints(cpDampedSpring *connection, Body *other,
                                    Vect point_on_this, Vect point_on_other,
                                    const SpringOptions &options);

    struct SimpleMotorOptions
    {
        float rate;
    };

    cpSimpleMotor *connectWithSimpleMotor(cpSimpleMotor *motor, Body *other,
                                          const SimpleMotorOptions &options);

    ///
    /// Remove this body from its internal space.
    ///
    void removeFromSpace();

  private:
    // functions
    void _add_to_space(Space *space);

    friend Shape;
    friend Space;
};
} // namespace lib
