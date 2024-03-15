#pragma once

#include <chipmunk/chipmunk_types.h>
#include <chipmunk/cpVect.h>
#include <raylib.h>

namespace lib
{
struct vect : public ::Vector2
{
    inline constexpr vect() noexcept
        :Vector2({0, 0})
    {
    }

    inline constexpr vect(cpVect c_version) noexcept
        :Vector2({c_version.x, c_version.y})
    {
    }

    inline constexpr vect(cpFloat cx, cpFloat cy) noexcept
        : Vector2({cx, cy})
    {
    }

    inline constexpr vect(float all) noexcept
        : Vector2({all, all})
    {
    }

    // static constructors
    static inline constexpr vect zero() noexcept { return vect(0, 0); }
    static inline constexpr vect forangle(float angle) noexcept
    {
        return cpvforangle(angle);
    }

    // math functions (do not modify the vector)
    [[nodiscard]] float magnitude() const noexcept;
    [[nodiscard]] float magnitude_sq() const noexcept;
    [[nodiscard]] float dot(const vect &other) const noexcept;
    [[nodiscard]] vect normalized() const noexcept;
    [[nodiscard]] vect perpendicular() const noexcept;
    [[nodiscard]] vect negative() const noexcept;
    [[nodiscard]] vect clamped(float length) const noexcept;
    [[nodiscard]] float cross(const vect &other) const noexcept;
    [[nodiscard]] float dist(const vect &other) const noexcept;

    // modifiers (change the vector and return nothing)
    void Negate() noexcept;
    void Clamp(float length) noexcept;
    void ClampComponentwise(const vect &lower, const vect &upper) noexcept;
    void Normalize() noexcept;
    void Scale(float scale) noexcept;
    void Round() noexcept;
    void Truncate() noexcept;

    inline constexpr bool operator==(const vect &other) const noexcept
    {
        return cpveql(*this, other);
    }
    inline constexpr vect operator-(const vect &other) const noexcept
    {
        return cpvsub(*this, other);
    }
    inline constexpr vect operator+(const vect &other) const noexcept
    {
        return vect(x + other.x, y + other.y);
    }
    inline constexpr vect operator*(float scale) const noexcept
    {
        return cpvmult(*this, scale);
    }
    inline constexpr vect operator/(float scale) const noexcept
    {
        return *this * (1 / scale);
    }
    inline constexpr vect operator/(const vect &other) const noexcept
    {
        vect nv = other;
        nv.x /= x;
        nv.y /= y;
        return nv;
    }
    inline constexpr vect operator*(const vect &other) const noexcept
    {
        return {other.x * x, other.y * y};
    }

    inline constexpr void operator/=(const vect &other) noexcept
    {
        x /= other.x;
        y /= other.y;
    }
    inline constexpr void operator*=(const vect &other) noexcept
    {
        x *= other.x;
        y *= other.y;
    }
    inline constexpr void operator-=(const vect &other) noexcept
    {
        x -= other.x;
        y -= other.y;
    }
    inline constexpr void operator+=(const vect &other) noexcept
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
