#pragma once
#include "allocators/stack.h"
#include "allocators/static_block.h"
#include <chipmunk/chipmunk_structs.h>
#include <stdbool.h>

typedef struct
{
    static_block_allocator_t bodies;
    static_block_allocator_t polys;
    static_block_allocator_t segments;
    cpSpace *space;
} physics_system_t;

void physics_system_update(physics_system_t *system);
// never returns null
cpBody *physics_system_static_body(physics_system_t *system);
// may return null
cpBody *physics_system_create_body(physics_system_t *system, float mass);

void physics_system_destroy_body(physics_system_t *system, cpBody *body);

typedef struct
{
    cpBody *body;
    cpVect a;
    cpVect b;
    float radius;
} physics_segment_shape_options_t;

cpSegmentShape *physics_system_create_segment_shape(
    physics_system_t *system, const physics_segment_shape_options_t *options);

typedef struct
{
    cpVect *vertices;
    u64 count;
} physics_vertices_t;

typedef struct
{
    cpBody *body;
    physics_vertices_t verts;
    cpTransform transform;
    float radius;
} physics_poly_shape_options_t;

typedef struct
{
    cpBody *body;
    cpBB bounding;
    float radius;
} physics_square_shape_options_t;

cpPolyShape *
physics_system_create_poly_shape(physics_system_t *system,
                                 const physics_poly_shape_options_t *options);

cpPolyShape *
physics_system_create_square(physics_system_t *system,
                             const physics_square_shape_options_t *options);

void physics_system_destroy_segment_shape(physics_system_t *system,
                                          cpSegmentShape *segment);

void physics_system_destroy_poly_shape(physics_system_t *system,
                                       cpPolyShape *poly);

void physics_system_register_singleton(physics_system_t *system);

physics_system_t *physics_system_instance();

bool physics_system_init(stack_allocator_t *allocator, physics_system_t *out);

cpDampedSpring *connect_with_damped_spring(cpDampedSpring *, cpBody *a,
                                           cpBody *b);

/// Free stuff that chipmunk mallocs internally and wont properly be removed
/// when we clear the level allocator.
void physics_system_free_chipmunk_extra_resources(physics_system_t *system);

void physics_system_unregister_singleton();
