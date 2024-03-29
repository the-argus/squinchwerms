#include "./global.h"
#include <stdlib.h>

u8 *global_allocator_alloc(u64 bytes) { return malloc(bytes); }

u8 *global_allocator_realloc(u8 *original, u64 new_bytes)
{
    return realloc(original, new_bytes);
}

void global_allocator_free(u8 *original) { free(original); }
