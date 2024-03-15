#pragma once
#include "chipmunk/chipmunk_structs.h"
#include "shape.h"
#include "vect.h"

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

    struct body_options_t
    {
        Type type;
        float mass;
        float moment;
    };

    inline Body(const body_options_t &options) : ::cpBody({})
    {
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
    void set_torque(float torque);
    [[nodiscard]] float moment() const;
    void set_moment(float moment);
    [[nodiscard]] Vect velocity() const;
    void set_velocity(Vect velocity);
    [[nodiscard]] Vect position() const;
    void set_position(Vect position);
    [[nodiscard]] Vect force() const;
    void set_force(Vect force);
    [[nodiscard]] float angle() const;
    void set_angle(float angle);
    [[nodiscard]] cpDataPointer user_data() const;
    void set_user_data(cpDataPointer data);

    void free();

    // read-only
    [[nodiscard]] Type type();

    struct spring_options_t
    {
        float length;
        float stiffness;
        float damping;
    };

    cpDampedSpring *connect_with_damped_spring(cpDampedSpring *connection,
                                               Body *other, Vect point_on_this,
                                               Vect point_on_other,
                                               const spring_options_t &options);
    cpDampedSpring *connect_with_damped_spring(cpDampedSpring *connection,
                                               Body *other,
                                               const spring_options_t &options);

    struct simple_motor_options_t
    {
        float rate;
    };

    cpSimpleMotor *
    connect_with_simple_motor(cpSimpleMotor *motor, Body *other,
                              const simple_motor_options_t &options);

    ///
    /// Remove this body from its internal space.
    ///
    void remove_from_space();

  private:
    // functions
    void _add_to_space(Space *space);

    friend Shape;
    friend Space;
};
} // namespace lib
