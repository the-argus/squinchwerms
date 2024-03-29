#include "stack.h"
#include "../allocators/global.h"
#include "../memory/align.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_BUFFER_SIZE 4096

bool stack_allocator_init(stack_allocator_t *out)
{
    void *firstbuf = global_allocator_alloc(STACK_BUFFER_SIZE);
    if (!firstbuf) {
        printf("ERROR: OOM\n");
        return false;
    }
    // only one buffer to start out
    pointer_collection_init(&out->buffers, 1);
    pointer_collection_append(&out->buffers, firstbuf);
    out->top_buffer = firstbuf;
    out->top_index = 0;
    return true;
}

void *stack_allocator_alloc(stack_allocator_t *ally, u64 bytes, u8 align)
{
    assert(bytes < STACK_BUFFER_SIZE);
    u8 *res = mem_align(ally->top_buffer + ally->top_index, align);
    // you cant 64 byte align a single byte. could be done but its rare and
    // usually a mistake so im putting an assert for it
    assert(bytes > align);
    assert(res >= ally->top_buffer + ally->top_index);
    u64 amount_alignment_moved = res - (ally->top_buffer + ally->top_index);
    assert(amount_alignment_moved <= bytes);

    // handle error case
    if (ally->top_index + amount_alignment_moved + bytes >= STACK_BUFFER_SIZE) {
        // need to make a new buffer
        u8 *newbuf = global_allocator_alloc(STACK_BUFFER_SIZE);
        if (!newbuf) {
            printf("FATAL: OOM\n");
            abort();
        }
        pointer_collection_append(&ally->buffers, newbuf);
        ally->top_buffer = newbuf;
        ally->top_index = 0;
        res = mem_align(ally->top_buffer + ally->top_index, align);
        assert(res >= ally->top_buffer + ally->top_index);
        u64 newtop = ally->top_index + amount_alignment_moved + bytes;
        assert(newtop < STACK_BUFFER_SIZE);
        ally->top_index = newtop;
        return ally->top_buffer;
    }

    // in this case, we can fit the object inside the buffer
    void *final_result =
        ally->top_buffer + ally->top_index + amount_alignment_moved;
    ally->top_index += amount_alignment_moved + bytes;
    return final_result;
}

void stack_allocator_deinit(stack_allocator_t *out)
{
    for (u64 i = 0; i < out->buffers.size; ++i) {
        void *buffer = pointer_collection_get(&out->buffers, i);
        global_allocator_free(buffer);
    }
#ifndef NDEBUG
    memset(out, 0, sizeof(stack_allocator_t));
#endif
}

void stack_allocator_clear(stack_allocator_t *ally)
{
    assert(ally->top_buffer);
    ally->top_buffer = pointer_collection_get(&ally->buffers, 0);
    ally->top_index = 0;
}
