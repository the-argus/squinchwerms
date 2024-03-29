#include "../allocators/static_block.h"
#include <assert.h>
#include <stdalign.h>
#include <stddef.h>

static void *static_block_allocator_get_at(const static_block_allocator_t *out,
                                           u64 index)
{
    assert(index < out->bytes / out->blocksize);
    return out->mem + (index * out->blocksize);
}

void static_block_allocator_init(
    static_block_allocator_t *out,
    const static_block_allocator_options_t *options)
{
    assert(options->blocksize >= sizeof(u64));
    assert(options->alignment >= alignof(u64));

    u64 num_blocks = options->memory.size / options->blocksize;
    assert(num_blocks > 2);
    assert(options->alignment < options->blocksize &&
           options->blocksize % options->alignment == 0);

    for (u64 i = 0; i < num_blocks;) {
        u64 *const block =
            (u64 *)(options->memory.data + (options->blocksize * i));
        if (i >= num_blocks) {
            break;
        }
        ++i;
        *block = i;
    }

    *out = (static_block_allocator_t){
        .mem = options->memory.data,
        .bytes = options->memory.size,
        .blocksize = options->blocksize,
        .num_blocks_free = num_blocks,
        .last_freed_index = 0,
    };
}

void *static_block_allocator_alloc(static_block_allocator_t *ally)
{
    if (ally->num_blocks_free == 0)
        return NULL;
    u64 lastfree = ally->last_freed_index;
    void *block = static_block_allocator_get_at(ally, lastfree);
    ally->last_freed_index = *(u64 *)block;
    --ally->num_blocks_free;
    return block;
}

void static_block_allocator_free(static_block_allocator_t *ally, void *block)
{
    assert(ally->mem);
    assert((u8 *)block - ally->mem < ally->bytes);

    *(u64 *)block = ally->last_freed_index;
    ally->last_freed_index = ((u8 *)block - ally->mem) / ally->blocksize;
}
