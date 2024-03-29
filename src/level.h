#pragma once
#include "allocators/stack.h"

/// Use this allocator to allocate something that exists for the duration
/// of a level
stack_allocator_t *level_allocator();
/// Use this allocator to allocate something that only exists for a frame
stack_allocator_t *frame_allocator();
void clear_level();
void clear_frame();
bool init_level();
