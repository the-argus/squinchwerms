// these two macros are needed so that we can allocate a lib::Space in an
// allocator
#define ALLO_ALLOW_NONTRIVIAL_COPY
#define ALLO_ALLOW_DESTRUCTORS
#include "physics.h"
#include "space.h"
#include <allo/block_allocator.h>
#include <allo/oneshot_allocator.h>
#include <ziglike/opt.h>
#include <ziglike/stdmem.h>
#include <ziglike/zigstdint.h>

static zl::opt<allo::oneshot_allocator_t> bodies_mem;
static zl::opt<allo::oneshot_allocator_t> polys_mem;
static zl::opt<allo::oneshot_allocator_t> segments_mem;
static zl::opt<allo::block_allocator_t> bodies;
static zl::opt<allo::block_allocator_t> polys;
static zl::opt<allo::block_allocator_t> segments;

static zl::opt<lib::Space &> space;

namespace werm {

void update_physics() noexcept { space.value().step(1 / 60.0f); }

BodyRef static_body() noexcept { return space.value().get_static_body(); }

allocation_status_t init_physics(allo::AllocatorDynRef parent) noexcept
{
    using namespace allo;

    auto res = parent.register_destruction_callback(
        [](void *) {
            bodies_mem.reset();
            bodies.reset();
            polys_mem.reset();
            polys.reset();
            segments_mem.reset();
            segments.reset();
        },
        nullptr);

    if (!res.okay())
        return res.err();

    {
        auto mem_bodies = alloc<u8>(parent, max_bodies * sizeof(lib::Body));
        if (!mem_bodies.okay())
            return mem_bodies.err();
        auto oneshot = oneshot_allocator_t::make(mem_bodies.release());
        if (!oneshot.okay())
            return oneshot.err();
        bodies_mem.emplace(std::move(oneshot.release()));

        auto bodies_res = block_allocator_t::make(
            bodies_mem.value().shoot(), bodies_mem.value(), sizeof(lib::Body));
        if (!bodies_res.okay())
            return bodies_res.err();
        bodies.emplace(std::move(bodies_res.release()));
    }

    {
        auto mem_polys = alloc<u8>(parent, max_shapes * sizeof(lib::PolyShape));
        if (!mem_polys.okay())
            return mem_polys.err();
        auto oneshot_polys = oneshot_allocator_t::make(mem_polys.release());
        if (!oneshot_polys.okay())
            return oneshot_polys.err();
        polys_mem.emplace(std::move(oneshot_polys.release()));

        auto polys_res =
            block_allocator_t::make(polys_mem.value().shoot(),
                                    polys_mem.value(), sizeof(lib::PolyShape));
        if (!polys_res.okay())
            return polys_res.err();
        polys.emplace(std::move(polys_res.release()));
    }

    {
        auto mem_segments =
            alloc<u8>(parent, max_shapes * sizeof(lib::SegmentShape));
        if (!mem_segments.okay())
            return mem_segments.err();
        auto oneshot_segments =
            oneshot_allocator_t::make(mem_segments.release());
        if (!oneshot_segments.okay())
            return oneshot_segments.err();
        segments_mem.emplace(std::move(oneshot_segments.release()));

        auto segments_res = block_allocator_t::make(
            segments_mem.value().shoot(), segments_mem.value(),
            sizeof(lib::SegmentShape));
        if (!segments_res.okay())
            return segments_res.err();
        segments.emplace(std::move(segments_res.release()));
    }

    auto spaceres = allo::construct_one<lib::Space>(parent);
    if (!spaceres.okay())
        return spaceres.err();

    space = spaceres.release();

    auto destroy_space = [](void *data) {
        ((lib::Space *)data)->lib::Space::~Space();
    };

    // unfortunately, chipmunk allocates other resources which the allocator
    // doesn't know about, so we have to hook in the cpSpaceDestroy function
    res = parent.register_destruction_callback(destroy_space, &space.value());

    if (!res.okay()) {
        destroy_space(&space.value());
        return res.err();
    }

    return AllocationStatusCode::Okay;
}

zl::opt<BodyRef> create_body(const lib::Body::body_options_t &options) noexcept
{
    auto mbody = allo::construct_one<lib::Body>(bodies.value(), options);
    if (!mbody.okay())
        return {};
    auto &body = mbody.release();
    space.value().add(body);
    return zl::opt<BodyRef>{std::in_place, &body};
}

void destroy_body(BodyRef ref) noexcept
{
    space.value().remove(*ref);
    ref->lib::Body::~Body();
    allo::free_one(bodies.value(), ref);
}

zl::opt<SegmentShapeRef>
create_segment_shape(BodyRef body,
                     const lib::SegmentShape::options_t &options) noexcept
{
    auto mshape = allo::construct_one<lib::SegmentShape>(segments.value(),
                                                         *body, options);
    if (!mshape.okay())
        return {};
    auto &shape = mshape.release();
    space.value().add(*shape.parent_cast());
    return zl::opt<SegmentShapeRef>{std::in_place, &shape};
}

zl::opt<PolyShapeRef>
create_poly_shape(BodyRef body,
                  const lib::PolyShape::default_options_t &options) noexcept
{
    auto mshape =
        allo::construct_one<lib::PolyShape>(polys.value(), *body, options);
    if (!mshape.okay())
        return {};
    auto &shape = mshape.release();
    space.value().add(*shape.parent_cast());
    return zl::opt<PolyShapeRef>{std::in_place, &shape};
}

// NOTE: identical code to create_poly_shape...
zl::opt<PolyShapeRef>
create_square(BodyRef body,
              const lib::PolyShape::square_options_t &options) noexcept
{
    auto mshape =
        allo::construct_one<lib::PolyShape>(polys.value(), *body, options);
    if (!mshape.okay())
        return {};
    return zl::opt<PolyShapeRef>{std::in_place, &mshape.release()};
}

void destroy_shape(ShapeRef ref) noexcept
{
    if (zl::memcontains(polys_mem.value().shoot(),
                        zl::raw_slice(*(u8 *)ref, 0))) {
        destroy_poly_shape(PolyShapeRef(ref));
    } else {
        destroy_segment_shape(SegmentShapeRef(ref));
    }
}
void destroy_poly_shape(PolyShapeRef ref) noexcept
{
    space.value().remove(*ref->parent_cast());
    ref->lib::PolyShape::~PolyShape();
    allo::free_one(polys.value(), ref);
}

void destroy_segment_shape(SegmentShapeRef ref) noexcept
{
    space.value().remove(*ref->parent_cast());
    ref->lib::SegmentShape::~SegmentShape();
    allo::free_one(segments.value(), ref);
}
} // namespace werm
