#pragma once
#include <allo.h>
#include <chipmunk/chipmunk_structs.h>

namespace werm {
    struct Body;
    struct Shape;
allocation_status_t init_physics(allo::AllocatorDynRef parent);


struct Body
{
    inline constexpr Body &operator->() noexcept { return ref; }

    inline constexpr ~Body() noexcept {  }

  private:
    Body &ref;
};


} // namespace werm
