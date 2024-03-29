#pragma once
#include "body.h"
#include "space.h"
#include <allo.h>
#include <allo/block_allocator.h>
#include <chipmunk/chipmunk_structs.h>
#include <ziglike/opt.h>

namespace werm {
using BodyRef = lib::Body *;
using ShapeRef = lib::Shape *;
using PolyShapeRef = lib::PolyShape *;
using SegmentShapeRef = lib::SegmentShape *;
using DampedSpringRef = cpDampedSpring *;

class PhysicsSystem
{
  public:
    static constexpr size_t max_bodies = 256;
    static constexpr size_t max_shapes = 256;

    static PhysicsSystem &singleton() noexcept;
    void register_singleton() noexcept;

    static zl::res<PhysicsSystem &, allo::AllocationStatusCode>
    make_with(allo::abstract_allocator_t &parent) noexcept;

    // allocate a damped spring and connect two bodies with it.
    zl::opt<DampedSpringRef> connect_with_damped_spring(
        allo::abstract_allocator_t &allocator, BodyRef a, BodyRef b,
        const lib::Body::spring_options_t &options) noexcept;

    void update() noexcept;

    BodyRef static_body() noexcept;

    zl::opt<BodyRef>
    create_body(const lib::Body::body_options_t &options) noexcept;

    void destroy_body(BodyRef) noexcept;

    zl::opt<SegmentShapeRef>
    create_segment_shape(BodyRef body,
                         const lib::SegmentShape::options_t &options) noexcept;

    zl::opt<PolyShapeRef> create_poly_shape(
        BodyRef body,
        const lib::PolyShape::default_options_t &options) noexcept;

    zl::opt<PolyShapeRef>
    create_square(BodyRef body,
                  const lib::PolyShape::square_options_t &options) noexcept;

    void destroy_shape(ShapeRef) noexcept;

    void destroy_poly_shape(PolyShapeRef) noexcept;

    void destroy_segment_shape(SegmentShapeRef) noexcept;

  private:
    struct M
    {
        allo::block_allocator_t bodies;
        allo::block_allocator_t polys;
        allo::block_allocator_t segments;
        zl::slice<uint8_t> polys_mem;
        lib::Space space;
    } m;
};
} // namespace werm
