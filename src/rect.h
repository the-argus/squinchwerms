#pragma once
#include "vect.h"
#include <chipmunk/cpBB.h>
#include <okay/short_arithmetic_types.h>
#include <raylib.h>

namespace lib {
struct Rect : public ::Rectangle
{
    constexpr Rect(cpBB chipmunk) noexcept
    {
        x = chipmunk.l;
        y = chipmunk.b;
        width = chipmunk.r - chipmunk.l;
        height = chipmunk.t - chipmunk.b;
    }
    constexpr Rect(Rectangle raylib) noexcept
    {
        x = raylib.x;
        y = raylib.x;
        width = raylib.width;
        height = raylib.height;
    }

    constexpr static Rect unitSquare() noexcept
    {
        return ::Rectangle{
            .x = 0.f,
            .y = 0.f,
            .width = 1.f,
            .height = 1.f,
        };
    }

    [[nodiscard]] constexpr Rect at(Vect coords) noexcept
    {
        return ::Rectangle{
            .x = coords.x,
            .y = coords.y,
            .width = width,
            .height = height,
        };
    }

    [[nodiscard]] constexpr Rect scaledBy(float scale) noexcept
    {
        return ::Rectangle{
            .x = x,
            .y = y,
            .width = width * scale,
            .height = height * scale,
        };
    }

    // conversion to chipmunk version
    [[nodiscard]] constexpr operator cpBB() const noexcept
    {
        float hw = width / 2.0f;
        float hh = height / 2.0f;
        return cpBB{.l = x - hw, .b = y - hh, .r = x + hw, .t = y + hh};
    }

    [[nodiscard]] constexpr bool equals(const Rect &other) const noexcept
    {
        return (x == other.x && y == other.y && width == other.width &&
                height == other.height);
    }

    [[nodiscard]] constexpr bool
    roughlyEquals(const Rect &other, float tol = 0.001f) const noexcept
    {
        return (fabsf(x - other.x) < tol) && (fabsf(y - other.y) < tol) &&
               (fabsf(width - other.width) < tol) &&
               (fabsf(height - other.height) < tol);
    }

    constexpr void draw(::Color color) const noexcept
    {
        ::DrawRectangleRec(*this, color);
    }

    constexpr void drawCentered(::Color color) const noexcept
    {
        ::DrawRectangleRec(
            {
                .x = x - (width / 2.f),
                .y = y - (height / 2.f),
                .width = width,
                .height = height,
            },
            color);
    }
};
} // namespace lib
