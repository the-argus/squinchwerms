#include "level.h"
#include <allo/heap_allocator.h>
#include <allo/reservation_allocator.h>
#include <allo/stack_allocator.h>
#include <ziglike/defer.h>
#include <ziglike/opt.h>
#include <ziglike/try.h>

static zl::opt<allo::heap_allocator_t> lvlheap;
static zl::opt<allo::stack_allocator_t> lvlstack;
static zl::opt<allo::reservation_allocator_t> lvlres_stack;
static zl::opt<allo::reservation_allocator_t> lvlres_heap;

namespace werm {
allo::HeapAllocatorDynRef level_heap() noexcept { return lvlheap.value(); }
allo::AllocatorDynRef level_allocator() noexcept { return lvlstack.value(); }
void clear_level() noexcept
{
    lvlheap.reset();
    lvlstack.reset();
}

allocation_status_t init_level() noexcept
{
    using namespace allo;
    TRY(static_reservation,
        reservation_allocator_t::make(
            {.committed = 1, .additional_pages_reserved = 20}));
    lvlres_stack.emplace(std::move(static_reservation));

    zl::defer remove_static([]() { lvlres_stack.reset(); });

    TRY(dynamic_reservation,
        reservation_allocator_t::make(
            {.committed = 1, .additional_pages_reserved = 20}));
    lvlres_heap.emplace(std::move(dynamic_reservation));

    zl::defer remove_dyn([]() { lvlres_heap.reset(); });

    TRY(heap, heap_allocator_t::make_owned(lvlres_heap.value().current_memory(),
                                           lvlres_heap.value()));
    lvlheap.emplace(std::move(heap));
    zl::defer remove_heap([]() { lvlheap.reset(); });

    TRY(stack, stack_allocator_t::make(lvlres_stack.value().current_memory(),
                                       lvlres_stack.value()));
    lvlstack.emplace(std::move(stack));

    remove_static.cancel();
    remove_dyn.cancel();
    remove_heap.cancel();

    return AllocationStatusCode::Okay;
}
} // namespace werm
