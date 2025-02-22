#include "vect.h"
#include "raymath.h"

namespace lib {
// math functions
float Vect::magnitude() const noexcept { return cpvlength(*this); }
float Vect::magnitudeSquared() const noexcept { return cpvlengthsq(*this); }
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
Vect &Vect::negateThis() noexcept
{
    Vect neg = negative();
    x = neg.x;
    y = neg.y;
    return *this;
}
Vect &Vect::clampThis(float length) noexcept
{
    *this = clamped(length);
    return *this;
}
Vect &Vect::normalizeThis() noexcept
{
    *this = normalized();
    return *this;
}
Vect &Vect::scaleThis(float scale) noexcept
{
    *this = *this * scale;
    return *this;
}
Vect &Vect::clampThisComponentwise(const lib::Vect &lower,
                                   const lib::Vect &upper) noexcept
{
    *this = ::Vector2Clamp(*this, lower, upper);
    return *this;
}
Vect &Vect::roundThis() noexcept
{
    x = std::round(x);
    y = std::round(y);
    return *this;
}
Vect &Vect::truncateThis() noexcept
{
    x = float(int(x));
    y = float(int(y));
    return *this;
}
} // namespace lib
