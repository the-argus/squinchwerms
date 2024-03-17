#pragma once
#include "body.h"
#include <allo.h>
#include <chipmunk/chipmunk_structs.h>
#include <ziglike/opt.h>

namespace werm {
constexpr size_t max_bodies = 256;
constexpr size_t max_shapes = 256;

using BodyRef = lib::Body *;
using ShapeRef = lib::Shape *;
using PolyShapeRef = lib::PolyShape *;
using SegmentShapeRef = lib::SegmentShape *;
using DampedSpringRef = cpDampedSpring *;

// allocate a damped spring and connect two bodies with it.
zl::opt<DampedSpringRef>
connect_with_damped_spring(allo::AllocatorDynRef allocator, BodyRef a,
                           BodyRef b,
                           const lib::Body::spring_options_t &options) noexcept;

void update_physics() noexcept;

BodyRef static_body() noexcept;

allo::allocation_status_t init_physics(allo::AllocatorDynRef parent) noexcept;

zl::opt<BodyRef> create_body(const lib::Body::body_options_t &options) noexcept;

void destroy_body(BodyRef) noexcept;

zl::opt<SegmentShapeRef>
create_segment_shape(BodyRef body,
                     const lib::SegmentShape::options_t &options) noexcept;

zl::opt<PolyShapeRef>
create_poly_shape(BodyRef body,
                  const lib::PolyShape::default_options_t &options) noexcept;

zl::opt<PolyShapeRef>
create_square(BodyRef body,
              const lib::PolyShape::square_options_t &options) noexcept;

void destroy_shape(ShapeRef) noexcept;

void destroy_poly_shape(PolyShapeRef) noexcept;

void destroy_segment_shape(SegmentShapeRef) noexcept;
} // namespace werm
