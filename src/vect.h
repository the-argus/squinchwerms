#pragma once

#include <chipmunk/chipmunk_types.h>
#include <chipmunk/cpVect.h>
#include <raylib.h>

namespace lib {
struct Vect : public ::Vector2
{
    inline constexpr Vect(Vector2 rl) noexcept : Vector2(rl) {}

    inline constexpr Vect() noexcept : Vector2({0, 0}) {}

    inline constexpr Vect(cpVect c_version) noexcept
        : Vector2({c_version.x, c_version.y})
    {
    }

    inline constexpr Vect(cpFloat cx, cpFloat cy) noexcept : Vector2({cx, cy})
    {
    }

    inline constexpr Vect(float all) noexcept : Vector2({all, all}) {}

    // static constructors
    static inline constexpr Vect zero() noexcept { return Vect(0, 0); }
    static inline constexpr Vect forangle(float angle) noexcept
    {
        return cpvforangle(angle);
    }

    // math functions (do not modify the Vector)
    [[nodiscard]] float magnitude() const noexcept;
    [[nodiscard]] float magnitude_sq() const noexcept;
    [[nodiscard]] float dot(const Vect &other) const noexcept;
    [[nodiscard]] Vect normalized() const noexcept;
    [[nodiscard]] Vect perpendicular() const noexcept;
    [[nodiscard]] Vect negative() const noexcept;
    [[nodiscard]] Vect clamped(float length) const noexcept;
    [[nodiscard]] float cross(const Vect &other) const noexcept;
    [[nodiscard]] float dist(const Vect &other) const noexcept;

    // modifiers (change the Vector and return nothing)
    void Negate() noexcept;
    void Clamp(float length) noexcept;
    void ClampComponentwise(const Vect &lower, const Vect &upper) noexcept;
    void Normalize() noexcept;
    void Scale(float scale) noexcept;
    void Round() noexcept;
    void Truncate() noexcept;

    inline constexpr bool operator==(const Vect &other) const noexcept
    {
        return cpveql(*this, other);
    }
    inline constexpr Vect operator-(const Vect &other) const noexcept
    {
        return cpvsub(*this, other);
    }
    inline constexpr Vect operator+(const Vect &other) const noexcept
    {
        return Vect(x + other.x, y + other.y);
    }
    inline constexpr Vect operator*(float scale) const noexcept
    {
        return cpvmult(*this, scale);
    }
    inline constexpr Vect operator/(float scale) const noexcept
    {
        return *this * (1 / scale);
    }
    inline constexpr Vect operator/(const Vect &other) const noexcept
    {
        Vect nv = other;
        nv.x /= x;
        nv.y /= y;
        return nv;
    }
    inline constexpr Vect operator*(const Vect &other) const noexcept
    {
        return {other.x * x, other.y * y};
    }

    inline constexpr void operator/=(const Vect &other) noexcept
    {
        x /= other.x;
        y /= other.y;
    }
    inline constexpr void operator*=(const Vect &other) noexcept
    {
        x *= other.x;
        y *= other.y;
    }
    inline constexpr void operator-=(const Vect &other) noexcept
    {
        x -= other.x;
        y -= other.y;
    }
    inline constexpr void operator+=(const Vect &other) noexcept
    {
        x += other.x;
        y += other.y;
    }

    inline constexpr operator cpVect() const noexcept
    {
        return cpVect{.x = x, .y = y};
    }
};
} // namespace lib
