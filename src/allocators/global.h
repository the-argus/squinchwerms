#pragma once
#include "intshorthand.h"

u8 *global_allocator_alloc(u64 bytes);

u8 *global_allocator_realloc(u8 *original, u64 new_bytes);

void global_allocator_free(u8 *original);
