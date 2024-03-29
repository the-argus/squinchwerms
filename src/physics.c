#include "physics.h"
#include <chipmunk/chipmunk.h>
#ifndef NDEBUG
#include <string.h>
#endif

#define PHYSICS_SYSTEM_MAX_BODIES 256
#define PHYSICS_SYSTEM_MAX_SHAPES 256

static physics_system_t *instance;

void physics_system_unregister_singleton() { instance = NULL; }

void physics_system_free_chipmunk_extra_resources(physics_system_t *system)
{
    cpSpaceDestroy(system->space);
}

bool physics_system_init(stack_allocator_t *allocator, physics_system_t *out)
{
    out->space = (cpSpace *)STACK_ALLOC_ONE(allocator, cpSpace).data;
    cpSpaceInit(out->space);
    // only (potential) failure point for this function
    // TODO: check if cpSpaceInit can even fail
    if (!out->space)
        return false;

    // these will only fail if the system runs out of memory, so no need to
    // bother recovering from that
    bytes_t bodies_mem =
        STACK_ALLOC(allocator, cpBody, PHYSICS_SYSTEM_MAX_BODIES);
    bytes_t polys_mem =
        STACK_ALLOC(allocator, cpPolyShape, PHYSICS_SYSTEM_MAX_SHAPES);
    bytes_t segments_mem =
        STACK_ALLOC(allocator, cpSegmentShape, PHYSICS_SYSTEM_MAX_SHAPES);

    // these initializers cannot fail
    static_block_allocator_init(
        &out->bodies,
        &(static_block_allocator_options_t){.memory = bodies_mem,
                                            .blocksize = sizeof(cpBody),
                                            .alignment = alignof(cpBody)});
    static_block_allocator_init(
        &out->polys,
        &(static_block_allocator_options_t){.memory = polys_mem,
                                            .blocksize = sizeof(cpPolyShape),
                                            .alignment = alignof(cpPolyShape)});
    static_block_allocator_init(&out->segments,
                                &(static_block_allocator_options_t){
                                    .memory = segments_mem,
                                    .blocksize = sizeof(cpSegmentShape),
                                    .alignment = alignof(cpSegmentShape)});

    return true;
}

void physics_system_update(physics_system_t *system)
{
    cpSpaceStep(system->space, 1 / 60.0f);
}

cpBody *physics_system_static_body(physics_system_t *system)
{
    return cpSpaceGetStaticBody(system->space);
}

// may return null
cpBody *physics_system_create_body(physics_system_t *system, float mass)
{
    cpBody *body = static_block_allocator_alloc(&system->bodies);
    if (!body) {
        return NULL;
    }
    cpBodyInit(body, mass, INFINITY);
    cpSpaceAddBody(system->space, body);
    return body;
}

void physics_system_destroy_body(physics_system_t *system, cpBody *body)
{
    cpSpaceRemoveBody(system->space, body);
    cpBodyDestroy(body);
#ifndef NDEBUG
    memset(body, 0, sizeof(cpBody));
#endif
    static_block_allocator_free(&system->bodies, body);
}

cpSegmentShape *physics_system_create_segment_shape(
    physics_system_t *system, const physics_segment_shape_options_t *options)
{
    assert(options->body);
    cpSegmentShape *seg = static_block_allocator_alloc(&system->segments);
    if (!seg)
        return NULL;
    cpSegmentShapeInit(seg, options->body, options->a, options->b,
                       options->radius);
    cpSpaceAddShape(system->space, &seg->shape);
    return seg;
}

cpPolyShape *
physics_system_create_poly_shape(physics_system_t *system,
                                 const physics_poly_shape_options_t *options)
{
    assert(options->body);
    assert(options->verts.vertices);
    cpPolyShape *poly = static_block_allocator_alloc(&system->polys);
    if (!poly)
        return NULL;
    cpPolyShapeInit(poly, options->body, (int)options->verts.count,
                    options->verts.vertices, options->transform,
                    options->radius);
    cpSpaceAddShape(system->space, &poly->shape);
    return poly;
}

cpPolyShape *
physics_system_create_square(physics_system_t *system,
                             const physics_square_shape_options_t *options)
{
    assert(options->body);
    assert(options->radius > 0.001f);
    cpPolyShape *poly = static_block_allocator_alloc(&system->polys);
    if (!poly)
        return NULL;
    cpBoxShapeInit2(poly, options->body, options->bounding, options->radius);
    cpSpaceAddShape(system->space, &poly->shape);
    return poly;
}

void physics_system_destroy_segment_shape(physics_system_t *system,
                                          cpSegmentShape *segment)
{
    cpSpaceRemoveShape(system->space, &segment->shape);
    cpShapeDestroy(&segment->shape);
    static_block_allocator_free(&system->segments, segment);
}

void physics_system_destroy_poly_shape(physics_system_t *system,
                                       cpPolyShape *poly)
{
    cpSpaceRemoveShape(system->space, &poly->shape);
    cpShapeDestroy(&poly->shape);
    static_block_allocator_free(&system->polys, poly);
}

void physics_system_register_singleton(physics_system_t *system)
{
    assert(!instance);
    instance = system;
}

physics_system_t *physics_system_instance() { return instance; }
