#pragma once
#include "body.h"
#include "shape.h"
#include "vect.h"
#include <chipmunk/chipmunk_structs.h>

namespace lib {
class Space : public ::cpSpace
{
  public:
    inline Space() noexcept : ::cpSpace({})
    {
        void *res = cpSpaceInit(this);
        assert(res);
    }
    inline ~Space() noexcept { cpSpaceDestroy(this); };

    Space(const Space &other) = delete;
    Space(Space &other) = delete;
    inline Space(const Space &&other) noexcept = delete;
    inline Space(Space &&other) = delete;

    void add(Body &body) noexcept;
    void add(Shape &shape) noexcept;
    void step(float timestep) noexcept;

    void remove(cpConstraint &constraint) noexcept;
    void remove(cpDampedSpring &constraint) noexcept;
    void remove(Shape &shape) noexcept;
    void remove(Body &body) noexcept;

    [[nodiscard]] float damping() const noexcept;
    void set_damping(float damping) noexcept;
    [[nodiscard]] Vect gravity() const noexcept;
    void set_gravity(Vect gravity) noexcept;
    [[nodiscard]] cpTimestamp collision_persistence() const noexcept;
    void set_collision_persistence(cpTimestamp persistence) noexcept;
    [[nodiscard]] cpDataPointer user_data() const noexcept;
    void set_user_data(cpDataPointer data) noexcept;
    void set_collision_bias(float bias) noexcept;
    [[nodiscard]] float collision_bias() const noexcept;
    void set_collision_slop(float slop) noexcept;
    [[nodiscard]] float collision_slop() const noexcept;
    void set_idle_speed_threshold(float threshold) noexcept;
    [[nodiscard]] float idle_speed_threshold() const noexcept;
    void set_iterations(int iterations) noexcept;
    [[nodiscard]] int iterations() const noexcept;

    [[nodiscard]] float get_sleep_time_threshold() const noexcept;
    [[nodiscard]] Body *get_static_body() noexcept;
    [[nodiscard]] float get_current_time_step() const noexcept;
};
} // namespace lib
