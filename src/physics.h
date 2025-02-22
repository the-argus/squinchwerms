#pragma once
#include "body.h"
#include "okay/allocators/allocator.h"
#include "space.h"
#include <chipmunk/chipmunk_structs.h>
#include <okay/opt.h>

namespace werm {

class PhysicsSystem
{
  public:
    static constexpr size_t max_bodies = 256;
    static constexpr size_t max_shapes = 256;

    static PhysicsSystem &singleton() noexcept;
    void register_singleton() noexcept;

    explicit PhysicsSystem() noexcept;

    // allocate a damped spring and connect two bodies with it.
    ok::opt_t<::cpDampedSpring &>
    connectWithDampedSpring(ok::allocator_t &allocator, lib::Body &a,
                            lib::Body &b,
                            const lib::Body::SpringOptions &options) noexcept;

    void update() noexcept;

    lib::Body &staticBody() noexcept;

    ok::opt_t<lib::Body &>
    createBody(const lib::Body::BodyOptions &options) noexcept;

    void destroy_body(lib::Body &) noexcept;

    ok::opt_t<lib::SegmentShape &>
    create_segment_shape(lib::Body &body,
                         const lib::SegmentShape::Options &options) noexcept;

    ok::opt_t<lib::PolyShape &>
    createPolyShape(lib::Body &body,
                    const lib::PolyShape::DefaultOptions &options) noexcept;

    ok::opt_t<lib::PolyShape &>
    createSquare(lib::Body &body,
                 const lib::PolyShape::SquareOptions &options) noexcept;

    void destroyShape(lib::Shape &) noexcept;

    void destroyPolyShape(lib::PolyShape &) noexcept;

    void destroySegmentShape(lib::SegmentShape &) noexcept;

  private:
    struct M
    {
        ok::allocator_t &bodies;
        ok::allocator_t &polys;
        ok::allocator_t &segments;
        ok::bytes_t polysMem;
        lib::Space space;
    } m;
};
} // namespace werm
