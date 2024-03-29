#include "pointer_collection.h"
#include "allocators/global.h"
#include <stdio.h>
#include <stdlib.h>

void *pointer_collection_get(const pointer_collection_t *list, u64 index)
{
    assert(index < list->size);
    return list->data[index];
}

void pointer_collection_set(pointer_collection_t *list, u64 index, void *value)
{
    assert(index < list->size);
    list->data[index] = value;
}

void pointer_collection_append(pointer_collection_t *list, void *value)
{
    if (list->capacity <= list->size + 1) {
        const u64 newcap =
            (u64)ceilf((float)list->size * POINTER_COLLECTION_REALLOC_RATIO);
        void **newmem = (void **)global_allocator_realloc(value, newcap);
        if (!newmem) {
            printf("FATAL: OOM");
            abort();
        }
        list->data = newmem;
        list->capacity = newcap;
    }

    list->data[list->size] = value;
    ++list->size;
}

void pointer_collection_remove(pointer_collection_t *list, u64 index)
{
    assert(index < list->size);
    list->data[index] = list->data[list->size - 1];
    --list->size;
}

void pointer_collection_init(pointer_collection_t *out, u64 initial)
{
    out->data = (void **)global_allocator_alloc(initial * sizeof(void *));
    out->capacity = initial;
    out->size = 0;
}
