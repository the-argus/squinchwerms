#pragma once
#include "intshorthand.h"
#include "assert.h"
#include <math.h>

#define POINTER_COLLECTION_REALLOC_RATIO 1.5f

/// A contiguous buffer of void pointers which can be inserted into or removed
/// from. Order is not preserved to allow for O(1) insertion and deletion.
typedef struct
{
    void **data;
    u64 size;
    u64 capacity;
} pointer_collection_t;

void pointer_collection_init(pointer_collection_t *out, u64 initial);

void *pointer_collection_get(const pointer_collection_t *list, u64 index);

void pointer_collection_set(pointer_collection_t *list, u64 index, void *value);

void pointer_collection_append(pointer_collection_t *list, void *value);

void pointer_collection_remove(pointer_collection_t *list, u64 index);
