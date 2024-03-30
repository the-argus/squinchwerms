#include "memory/align.h"

u8 *mem_align_exponent(u8 *input, u8 align_exponent) // NOLINT
{
    u8 current = get_alignment_exponent_from_alignment((u64)input);
    if (current >= align_exponent)
        return input;
    assert(align_exponent < 10);
    u64 res = (((u64)input >> align_exponent) + 1) << align_exponent;
    assert(res >= (u64)input);
#ifndef NDEBUG
    u64 diff = res - (u64)input;
    u64 maskplusone = (1UL << align_exponent);
    assert(diff <= maskplusone);
#endif
    return (u8 *)res; // NOLINT
}

u8 get_alignment_exponent_from_alignment(u64 alignment)
{
    u8 bits = sizeof(u64) * 8;
    u64 mask = 1;
    for (u64 i = 0; i < bits; ++i) {
        if ((mask & alignment) != 0) {
            return i;
        }
        mask = mask << 1;
        mask += 1;
    }
    assert(0);
    // should only happen on 0 address
    return bits;
}

u8 *mem_align(u8 *addr, u8 alignment)
{
    return mem_align_exponent(addr,
                              get_alignment_exponent_from_alignment(alignment));
}
