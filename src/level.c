#include "level.h"
#include <allo/heap_allocator.h>
#include <allo/oneshot_allocator.h>
#include <allo/stack_allocator.h>
#include <ziglike/defer.h>
#include <ziglike/opt.h>
#include <ziglike/zigstdint.h>

static allo::c_allocator_t parent;
static zl::opt<allo::heap_allocator_t> lvlheap;
static zl::opt<allo::stack_allocator_t> lvlstack;

namespace werm {
allo::HeapAllocatorDynRef level_heap() noexcept { return lvlheap.value(); }
allo::AllocatorDynRef level_allocator() noexcept { return lvlstack.value(); }
void clear_level() noexcept
{
    lvlheap.reset();
    lvlstack.reset();
}

allo::allocation_status_t init_level() noexcept
{
    using namespace allo;
    {
        auto mem_res = allo::alloc<u8>(parent, 200UL * 4096);
        if (!mem_res.okay())
            return mem_res.err();
        zl::slice<u8> mem = mem_res.release();

        auto heap_res = heap_allocator_t::make_owned(mem, parent);
        if (!heap_res.okay()) {
            allo::free(parent, mem);
            return heap_res.err();
        }
        lvlheap.emplace(std::move(heap_res.release()));
    }

    zl::defer remove_heap([]() { lvlheap.reset(); });

    {
        auto mem_res = allo::alloc<u8>(parent, 200UL * 4096);
        if (!mem_res.okay())
            return mem_res.err();
        zl::slice<u8> mem = mem_res.release();

        auto stack_res = stack_allocator_t::make(mem, parent);
        if (!stack_res.okay()) {
            allo::free(parent, mem);
            return stack_res.err();
        }
        lvlstack.emplace(std::move(stack_res.release()));
    }

    remove_heap.cancel();

    return AllocationStatusCode::Okay;
}
} // namespace werm
