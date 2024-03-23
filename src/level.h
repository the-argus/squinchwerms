#pragma once
#include <allo.h>
#include <allo/status.h>
#include <ziglike/anystatus.h>

namespace werm {
allo::abstract_heap_allocator_t &level_heap() noexcept;
allo::abstract_allocator_t &level_allocator() noexcept;
allo::abstract_allocator_t &frame_allocator() noexcept;
void clear_level() noexcept;
void clear_frame() noexcept;
allo::allocation_status_t init_level() noexcept;
} // namespace werm
