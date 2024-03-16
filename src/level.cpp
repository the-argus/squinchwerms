#include "level.h"
#include "natural_log/natural_log.h"
#include <allo/heap_allocator.h>
#include <allo/reservation_allocator.h>
#include <allo/stack_allocator.h>
#include <ziglike/defer.h>
#include <ziglike/opt.h>

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
    lvlres_stack.reset();
    lvlres_heap.reset();
}

allocation_status_t init_level() noexcept
{
    using namespace allo;
    auto static_reservation = reservation_allocator_t::make(
        {.committed = 1, .additional_pages_reserved = 20});
    if (!static_reservation.okay())
        return static_reservation.err();
    lvlres_stack.emplace(std::move(static_reservation.release()));

    zl::defer remove_static([]() {
        lvlres_stack.reset();
        LN_DEBUG("Failure, deleting static memory reservation for level.");
    });

    auto dynamic_reservation = reservation_allocator_t::make(
        {.committed = 1, .additional_pages_reserved = 20});
    if (!dynamic_reservation.okay())
        return dynamic_reservation.err();
    lvlres_heap.emplace(std::move(dynamic_reservation.release()));

    zl::defer remove_dyn([]() {
        lvlres_heap.reset();
        LN_DEBUG("Failure, deleting dynamic memory reservation for level.");
    });

    auto heap_res = heap_allocator_t::make_owned(
        lvlres_heap.value().current_memory(), lvlres_heap.value());
    if (!heap_res.okay())
        return heap_res.err();
    lvlheap.emplace(std::move(heap_res.release()));
    zl::defer remove_heap([]() {
        lvlheap.reset();
        LN_DEBUG("Failure, deleting heap allocator");
    });

    auto stack_res = stack_allocator_t::make(
        lvlres_stack.value().current_memory(), lvlres_stack.value());
    if (!stack_res.okay())
        return stack_res.err();
    lvlstack.emplace(std::move(stack_res.release()));

    remove_static.cancel();
    remove_dyn.cancel();
    remove_heap.cancel();

    return AllocationStatusCode::Okay;
}
} // namespace werm
