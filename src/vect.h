#pragma once

#include <chipmunk/chipmunk_types.h>
#include <chipmunk/cpVect.h>
#include <fmt/core.h>
#include <raylib.h>

namespace lib {
struct zeroed_tag
{};

struct Vect : public ::Vector2
{
    constexpr Vect(Vector2 rl) noexcept : Vector2(rl) {}

    constexpr Vect() noexcept : Vector2({0, 0}) {}

    constexpr Vect(cpVect c_version) noexcept
        : Vector2({c_version.x, c_version.y})
    {
    }

    constexpr Vect(cpFloat cx, cpFloat cy) noexcept : Vector2({cx, cy}) {}

    constexpr Vect(float all) noexcept : Vector2({all, all}) {}

    // static constructors
    static constexpr Vect zero() noexcept { return Vect(0, 0); }
    static constexpr Vect forAngle(float angle) noexcept
    {
        return Vect(cosf(angle), sinf(angle));
    }

    // math functions (do not modify the Vector)
    [[nodiscard]] float magnitude() const noexcept;
    [[nodiscard]] float magnitudeSquared() const noexcept;
    [[nodiscard]] float dot(const Vect &other) const noexcept;
    [[nodiscard]] Vect normalized() const noexcept;
    [[nodiscard]] Vect perpendicular() const noexcept;
    [[nodiscard]] Vect negative() const noexcept;
    [[nodiscard]] Vect clamped(float length) const noexcept;
    [[nodiscard]] float cross(const Vect &other) const noexcept;
    [[nodiscard]] float dist(const Vect &other) const noexcept;

    // modifiers (change the Vector and return nothing)
    Vect &negateThis() noexcept;
    Vect &clampThis(float length) noexcept;
    Vect &clampThisComponentwise(const Vect &lower, const Vect &upper) noexcept;
    Vect &normalizeThis() noexcept;
    Vect &scaleThis(float scale) noexcept;
    Vect &roundThis() noexcept;
    Vect &truncateThis() noexcept;

    constexpr bool isRoughlyEquals(const Vect &other) const noexcept
    {
        auto out = (other - *this);
        return fabsf(out.x) < 0.001 && fabsf(out.y) < 0.001;
    }

    constexpr bool operator==(const Vect &other) const noexcept
    {
        return this->x == other.x && this->y == other.y;
    }
    constexpr Vect operator-(const Vect &other) const noexcept
    {
        return cpVect(x - other.x, y - other.y);
    }
    constexpr Vect operator+(const Vect &other) const noexcept
    {
        return Vect(x + other.x, y + other.y);
    }
    constexpr Vect operator*(float scale) const noexcept
    {
        return Vect(x * scale, y * scale);
    }
    constexpr Vect operator/(float scale) const noexcept
    {
        return *this * (1 / scale);
    }
    constexpr Vect operator/(const Vect &other) const noexcept
    {
        Vect nv = other;
        nv.x /= x;
        nv.y /= y;
        return nv;
    }
    constexpr Vect operator*(const Vect &other) const noexcept
    {
        return {other.x * x, other.y * y};
    }

    constexpr Vect &operator/=(const Vect &other) noexcept
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }
    constexpr Vect &operator*=(const Vect &other) noexcept
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }
    constexpr Vect &operator-=(const Vect &other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    constexpr Vect &operator+=(const Vect &other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr operator cpVect() const noexcept
    {
        return cpVect{.x = x, .y = y};
    }
};
} // namespace lib

template <> struct fmt::formatter<lib::Vect>
{
    constexpr format_parse_context::iterator parse(format_parse_context &ctx)
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");
        return it;
    }

    format_context::iterator format(const lib::Vect &v,
                                    format_context &ctx) const
    {
        return fmt::format_to(ctx.out(), "({} {})", v.x, v.y);
    }
};
