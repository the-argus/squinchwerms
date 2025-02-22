#pragma once
#include "vect.h"
#include <chipmunk/cpBB.h>
#include <raylib.h>

namespace lib {
struct Rect : public ::Rectangle
{
    constexpr Rect()
    {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
    }

    constexpr Rect(float x, float y, float width, float height)
        : Rect(Vect{x, y}, Vect{width, height})
    {
    }

    constexpr Rect(Vect position, Vect size)
    {
        x = position.x;
        y = position.y;
        width = size.x;
        height = size.y;
    }

    // conversion TO original types (implicit)
    constexpr Rect(cpBB chipmunk)
    {
        width = chipmunk.r - chipmunk.l;
        height = chipmunk.t - chipmunk.b;
        x = chipmunk.l;
        y = chipmunk.b;
    }
    constexpr Rect(Rectangle raylib)
    {
        x = raylib.x;
        y = raylib.x;
        width = raylib.width;
        height = raylib.height;
    }

    // conversion to chipmunk version
    constexpr operator cpBB() const noexcept
    {
        float hw = width / 2.0f;
        float hh = height / 2.0f;
        return cpBB{.l = x - hw, .b = y - hh, .r = x + hw, .t = y + hh};
    }

    constexpr bool operator==(const Rect &other) const
    {
        return (x == other.x && y == other.y && width == other.width &&
                height == other.height);
    }

    inline void draw(::Color color) { ::DrawRectangleRec(*this, color); }
};
} // namespace lib
