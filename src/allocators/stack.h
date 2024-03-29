#pragma once
#include "bytes.h"
#include "structures/pointer_collection.h"
#include <stdalign.h>
#include <stdbool.h>

typedef struct
{
    pointer_collection_t buffers;
    u8 *top_buffer;
    u16 top_index;
} stack_allocator_t;

bool stack_allocator_init(stack_allocator_t *out);

void stack_allocator_deinit(stack_allocator_t *out);

bytes_t stack_allocator_alloc(stack_allocator_t *ally, u64 bytes, u8 align);

// TODO:
// BUG:
// this leaves old buffers unused. change stack_allocator_alloc so that it
// checks for already allocated buffers instead of always appending new
// ones
void stack_allocator_clear(stack_allocator_t *ally);

#define STACK_ALLOC_ONE(allocator, type) \
    (type *)stack_allocator_alloc(allocator, sizeof(type), alignof(type)).data
#define STACK_ALLOC(allocator, type, amount) \
    stack_allocator_alloc(allocator, (amount) * sizeof(type), alignof(type))
