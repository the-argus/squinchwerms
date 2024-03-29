#include "vect.h"
#include "raymath.h"

namespace lib {
// math functions
float Vect::magnitude() const noexcept { return cpvlength(*this); }
float Vect::magnitude_sq() const noexcept { return cpvlengthsq(*this); }
Vect Vect::perpendicular() const noexcept { return cpvperp(cpv(x, y)); }
Vect Vect::negative() const noexcept { return cpvneg(cpv(x, y)); }
float Vect::dot(const Vect &other) const noexcept
{
    return cpvdot(*this, other);
}
Vect Vect::normalized() const noexcept { return cpvnormalize(*this); }
Vect Vect::clamped(float length) const noexcept
{
    return cpvclamp(*this, length);
}
float Vect::cross(const Vect &other) const noexcept
{
    return cpvcross(*this, other);
}
float GetDist(Vect other);

// modifiers
void Vect::Negate() noexcept
{
    Vect neg = negative();
    x = neg.x;
    y = neg.y;
}
void Vect::Clamp(float length) noexcept { *this = clamped(length); }
void Vect::Normalize() noexcept { *this = normalized(); }
void Vect::Scale(float scale) noexcept { *this = *this * scale; }
void Vect::ClampComponentwise(const lib::Vect &lower,
                              const lib::Vect &upper) noexcept
{
    *this = ::Vector2Clamp(*this, lower, upper);
}
void Vect::Round() noexcept
{
    x = std::round(x);
    y = std::round(y);
}
void Vect::Truncate() noexcept
{
    x = float(int(x));
    y = float(int(y));
}
} // namespace lib
