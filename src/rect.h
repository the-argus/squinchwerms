#pragma once
#include <assert.h>
#include <chipmunk/cpBB.h>
#include <raylib.h>

inline Rectangle cpbb_to_raylib(cpBB b)
{
    assert(b.r > b.l);
    assert(b.t < b.b);
    return (Rectangle){
        .x = (b.l + b.r) / 2,
        .y = (b.t + b.b) / 2,
        .width = fabsf(b.r - b.l),
        .height = fabsf(b.b - b.t),
    };
}
