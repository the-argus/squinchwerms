// these two macros are needed so that we can allocate a lib::Space in an
// allocator
#include "ziglike/defer.h"
#define ALLO_ALLOW_NONTRIVIAL_COPY
#define ALLO_ALLOW_DESTRUCTORS
#include "natural_log/natural_log.h"
#include "physics.h"
#include <ziglike/opt.h>
#include <ziglike/stdmem.h>
#include <ziglike/zigstdint.h>

werm::PhysicsSystem *physics_singleton;

namespace werm {

static PhysicsSystem &singleton() noexcept
{
#ifndef NDEBUG
    if (!physics_singleton) {
        LN_FATAL("No physics singleton registered, unable to perform physics "
                 "operation");
    }
#endif
    return *physics_singleton;
}

void PhysicsSystem::register_singleton() noexcept { physics_singleton = this; }

void PhysicsSystem::update() noexcept { m.space.step(1 / 60.0f); }

BodyRef PhysicsSystem::static_body() noexcept
{
    return m.space.get_static_body();
}

zl::res<PhysicsSystem &, allo::AllocationStatusCode>
PhysicsSystem::make(allo::AllocatorDynRef parent) noexcept
{
    using namespace allo;
    auto final_res = alloc_one<PhysicsSystem>(parent);
    if (!final_res.okay())
        return final_res.err();
    // NOTE: the contents of this physics system is uninitialized at this point!
    werm::PhysicsSystem &final = final_res.release();

    // create the block allocator for bodies
    {
        auto mem_bodies_res = alloc<u8>(parent, max_bodies * sizeof(lib::Body));
        if (!mem_bodies_res.okay())
            return mem_bodies_res.err();

        auto block_bodies_res = block_allocator_t::make(
            mem_bodies_res.release(), sizeof(lib::Body));
        if (!block_bodies_res.okay())
            return block_bodies_res.err();
        new (&final.m.bodies)
            block_allocator_t(std::move(block_bodies_res.release()));
    }

    // create the block allocator for poly shapes
    {
        auto mem_polys_res =
            alloc<u8>(parent, max_shapes * sizeof(lib::PolyShape));
        if (!mem_polys_res.okay())
            return mem_polys_res.err();
        auto mem_polys = mem_polys_res.release();
        auto polys_res =
            block_allocator_t::make(mem_polys, sizeof(lib::PolyShape));
        if (!polys_res.okay())
            return polys_res.err();
        new (&final.m.polys) block_allocator_t(std::move(polys_res.release()));
        new (&final.m.polys_mem) zl::slice<uint8_t>(mem_polys);
    }

    // create the block allocator for segment shapes
    {
        auto mem_segments =
            alloc<u8>(parent, max_shapes * sizeof(lib::SegmentShape));
        if (!mem_segments.okay())
            return mem_segments.err();

        auto segments_res = block_allocator_t::make(mem_segments.release(),
                                                    sizeof(lib::SegmentShape));
        if (!segments_res.okay())
            return segments_res.err();
        new (&final.m.segments)
            block_allocator_t(std::move(segments_res.release()));
    }

    new (&final.m.space) lib::Space();

    auto destroy_space = [](void *data) {
        ((lib::Space *)data)->lib::Space::~Space();
    };

    // unfortunately, chipmunk allocates other resources which the allocator
    // doesn't know about, so we have to hook in the cpSpaceDestroy function
    auto res =
        parent.register_destruction_callback(destroy_space, &final.m.space);

    if (!res.okay()) {
        destroy_space(&final.m.space);
        return res.err();
    }

    return final;
}

zl::opt<DampedSpringRef> PhysicsSystem::connect_with_damped_spring(
    allo::AllocatorDynRef allocator, BodyRef a, BodyRef b,
    const lib::Body::spring_options_t &options) noexcept
{
    auto res = allo::alloc_one<cpDampedSpring>(allocator);
    if (!res.okay())
        return {};
    DampedSpringRef result = &res.release();
    a->connect_with_damped_spring(result, b, options);
    return result;
}

zl::opt<BodyRef>
PhysicsSystem::create_body(const lib::Body::body_options_t &options) noexcept
{
    auto mbody = allo::construct_one<lib::Body>(m.bodies, options);
    if (!mbody.okay())
        return {};
    auto &body = mbody.release();
    m.space.add(body);
    return zl::opt<BodyRef>{std::in_place, &body};
}

void PhysicsSystem::destroy_body(BodyRef ref) noexcept
{
    m.space.remove(*ref);
    ref->free();
    ref->lib::Body::~Body();
    allo::free_one(m.bodies, ref);
}

zl::opt<SegmentShapeRef> PhysicsSystem::create_segment_shape(
    BodyRef body, const lib::SegmentShape::options_t &options) noexcept
{
    auto mshape =
        allo::construct_one<lib::SegmentShape>(m.segments, *body, options);
    if (!mshape.okay())
        return {};
    auto &shape = mshape.release();
    m.space.add(*shape.parent_cast());
    return zl::opt<SegmentShapeRef>{std::in_place, &shape};
}

zl::opt<PolyShapeRef> PhysicsSystem::create_poly_shape(
    BodyRef body, const lib::PolyShape::default_options_t &options) noexcept
{
    auto mshape = allo::construct_one<lib::PolyShape>(m.polys, *body, options);
    if (!mshape.okay())
        return {};
    auto &shape = mshape.release();
    m.space.add(*shape.parent_cast());
    return zl::opt<PolyShapeRef>{std::in_place, &shape};
}

// NOTE: identical code to create_poly_shape...
zl::opt<PolyShapeRef> PhysicsSystem::create_square(
    BodyRef body, const lib::PolyShape::square_options_t &options) noexcept
{
    auto mshape = allo::construct_one<lib::PolyShape>(m.polys, *body, options);
    if (!mshape.okay())
        return {};
    auto &shape = mshape.release();
    m.space.add(*shape.parent_cast());
    return zl::opt<PolyShapeRef>{std::in_place, &shape};
}

void PhysicsSystem::destroy_shape(ShapeRef ref) noexcept
{
    if (zl::memcontains(m.polys_mem, zl::raw_slice(*(u8 *)ref, 0))) {
        destroy_poly_shape(PolyShapeRef(ref));
    } else {
        destroy_segment_shape(SegmentShapeRef(ref));
    }
}
void PhysicsSystem::destroy_poly_shape(PolyShapeRef ref) noexcept
{
    m.space.remove(*ref->parent_cast());
    ref->parent_cast()->free();
    ref->lib::PolyShape::~PolyShape();
    allo::free_one(m.polys, ref);
}

void PhysicsSystem::destroy_segment_shape(SegmentShapeRef ref) noexcept
{
    m.space.remove(*ref->parent_cast());
    ref->parent_cast()->free();
    ref->lib::SegmentShape::~SegmentShape();
    allo::free_one(m.segments, ref);
}
} // namespace werm
