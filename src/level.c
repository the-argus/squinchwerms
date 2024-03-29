#include "level.h"

static stack_allocator_t level_ally;
static stack_allocator_t frame_ally;

stack_allocator_t *level_allocator()
{
    assert(level_ally.top_buffer);
    return &level_ally;
}

stack_allocator_t *frame_allocator()
{
    assert(frame_ally.top_buffer);
    return &frame_ally;
}

void clear_level() { stack_allocator_clear(&level_ally); }

void clear_frame() { stack_allocator_clear(&frame_ally); }

bool init_level()
{
    if (!stack_allocator_init(&level_ally)) {
        return false;
    }
    if (!stack_allocator_init(&frame_ally)) {
        stack_allocator_deinit(&level_ally);
        return false;
    }
    return true;
}
