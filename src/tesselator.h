#pragma once

#include "rect.h"
#include "vect.h"
#include <cassert>
#include <okay/macros/foreach.h>
#include <okay/opt.h>
#include <vector>

namespace lib::tesselate {

/// taken from
/// https://github.com/k-j0/haze-triangulator/blob/master/Haze/Triangulator/Scripts/Triangulator.cs

struct Triangle
{
    Vect a;
    Vect b;
    Vect c;
};

struct Segment
{
    Vect a;
    Vect b;

    /** Utility to get the center point of the line segment. */
    [[nodiscard]] constexpr Vector2 center() { return (a + b) * 0.5f; }
};

constexpr void rotatePathClockwise(std::vector<Vect> &path)
{
    assert(!path.empty());
    path.push_back(path[0]);
    path.erase(path.begin());
}

ok::opt_t<Vect> lineSegmentsIntersection(Vect p1, Vect p2, Vect p3, Vect p4,
                                         bool allowOnLine = true) noexcept;

Rect aabbFromPath(ok::slice_t<const Vect> path) noexcept;

bool isPointInsidePolygon(ok::slice_t<const Vect> polygon, Vect point) noexcept;

bool isPolygonClockwise(ok::slice_t<const lib::Vect> polygon) noexcept;

bool isTriangleClockwise(Triangle tri) noexcept;

Triangle certifyWindingOrder(Triangle tri, bool clockwise) noexcept;

void certifyWindingOrder(ok::slice_t<Triangle> tris, bool clockwise) noexcept;

ok::opt_t<std::vector<Triangle>> triangulate(ok::slice_t<const Vect> pathSource,
                                             int subsequentCalls = 0);

void addVertexToMesh(std::vector<::Vector3> &vertices,
                     std::vector<u16> &indices, Vector3 vertex) noexcept;

void addTriangleToMesh(std::vector<Vector3> &vertices,
                       std::vector<u16> &indices, Triangle tri, float z,
                       bool clockwise) noexcept;

void addTrianglesToMesh(std::vector<Vector3> &vertices,
                        std::vector<u16> &indices,
                        ok::slice_t<const Triangle> tris, float z,
                        bool clockwise) noexcept;
} // namespace lib::tesselate
