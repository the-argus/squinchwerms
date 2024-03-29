#pragma once
#include "bytes.h"
#include "intshorthand.h"

typedef struct
{
    u8 *mem;
    u64 bytes;
    u64 last_freed_index;
    u64 num_blocks_free;
    u64 blocksize;
} static_block_allocator_t;

typedef struct
{
    bytes_t memory;
    u64 blocksize;
    u8 alignment;
} static_block_allocator_options_t;

/// Construct a static block allocator. The memory given must be at least
/// large enough for 2x blocksize and be aligned at least as much as the
/// requested alignment for the items. Blocksize must be greater than 8 bytes
/// A static block allocator does not need to be deinitialized.
void static_block_allocator_init(
    static_block_allocator_t *out,
    const static_block_allocator_options_t *options);

void *static_block_allocator_alloc(static_block_allocator_t *ally);

void static_block_allocator_free(static_block_allocator_t *ally, void *block);
