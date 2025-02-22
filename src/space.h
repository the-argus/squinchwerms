#pragma once
#include "body.h"
#include "shape.h"
#include "vect.h"
#include <chipmunk/chipmunk_structs.h>
#include <cstring>

namespace lib {
class Space : public ::cpSpace
{
  public:
    constexpr Space() noexcept : ::cpSpace({})
    {
        std::memset(static_cast<cpSpace *>(this), 0, sizeof(cpSpace));
        void *res = cpSpaceInit(this);
        assert(res);
    }

    constexpr void destroy() noexcept { cpSpaceDestroy(this); }

    Space(const Space &other) = delete;
    Space &operator=(const Space &other) = delete;
    Space(Space &&other) noexcept = delete;
    Space &operator=(Space &&other) noexcept = delete;

    void add(Body &body) noexcept;
    void add(Shape &shape) noexcept;
    void step(float timestep) noexcept;

    void remove(cpConstraint &constraint) noexcept;
    void remove(cpDampedSpring &constraint) noexcept;
    void remove(Shape &shape) noexcept;
    void remove(Body &body) noexcept;

    [[nodiscard]] float damping() const noexcept;
    Space &setDamping(float damping) noexcept;
    [[nodiscard]] Vect gravity() const noexcept;
    Space &setGravity(Vect gravity) noexcept;
    [[nodiscard]] cpTimestamp collisionPersistence() const noexcept;
    Space &setCollisionPersistence(cpTimestamp persistence) noexcept;
    [[nodiscard]] cpDataPointer userData() const noexcept;
    Space &setUserData(cpDataPointer data) noexcept;
    Space &setCollisionBias(float bias) noexcept;
    [[nodiscard]] float collisionBias() const noexcept;
    Space &setCollisionSlop(float slop) noexcept;
    [[nodiscard]] float collisionSlop() const noexcept;
    Space &setIdleSpeedThreshold(float threshold) noexcept;
    [[nodiscard]] float idleSpeedThreshold() const noexcept;
    Space &setIterations(int iterations) noexcept;
    [[nodiscard]] int iterations() const noexcept;

    [[nodiscard]] float getSleepTimeThreshold() const noexcept;
    [[nodiscard]] Body &getStaticBody() noexcept;
    [[nodiscard]] float getCurrentTimeStep() const noexcept;
};
} // namespace lib
