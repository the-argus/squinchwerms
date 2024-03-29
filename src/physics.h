#pragma once
#include "allocators/static_block.h"
#include <chipmunk/chipmunk_structs.h>
#include <stdbool.h>

#define PHYSICS_SYSTEM_MAX_BODIES 256
#define PHYSICS_SYSTEM_MAX_SHAPES 256

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
cpBody *physics_system_create_body(physics_system_t *system);

void physics_system_destroy_body(cpBody *body);

cpSegmentShape *physics_system_create_segment_shape(physics_system_t *system);

cpPolyShape *physics_system_create_poly_shape(physics_system_t *system);

cpPolyShape *physics_system_create_square(physics_system_t *system);

void physics_system_destroy_segment_shape(cpSegmentShape *segment);

void physics_system_destroy_poly_shape(cpPolyShape *poly);

void physics_system_register_singleton(physics_system_t *system);

physics_system_t *physics_system_instance();

bool physics_system_init(physics_system_t *out);

cpDampedSpring *connect_with_damped_spring(cpDampedSpring *, cpBody *a,
                                           cpBody *b);
