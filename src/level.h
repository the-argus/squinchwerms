#pragma once
#include <allo.h>
#include <allo/status.h>
#include <ziglike/anystatus.h>

namespace werm {
allo::HeapAllocatorDynRef level_heap() noexcept;
allo::AllocatorDynRef level_allocator() noexcept;
allo::AllocatorDynRef frame_allocator() noexcept;
void clear_level() noexcept;
void clear_frame() noexcept;
allo::allocation_status_t init_level() noexcept;
} // namespace werm
