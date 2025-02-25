#pragma once

#include "body.h"
#include "shape.h"
#include <okay/allocators/allocator.h>
#include <okay/short_arithmetic_types.h>

namespace lib {

struct SerializationVersion
{
    u64 maj;
    u64 min;
    u64 patch;
};

using body_allocator_t = Body *(*)();
using poly_shape_allocator_t = PolyShape *(*)();
using segment_shape_allocator_t = SegmentShape *(*)();

struct PhysicsAllocators
{
    body_allocator_t bodyAllocator;
    poly_shape_allocator_t polyShapeAllocator;
    segment_shape_allocator_t segmentShapeAllocator;
	ok::allocator_t& vertexBufScratch;
};

struct Space;
bool deserializeSpace(Space &uninitialized, const char *filepath,
                      const PhysicsAllocators &allocators) noexcept;
} // namespace lib
