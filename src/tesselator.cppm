#include "tesselator.h"
#include "natural_log/natural_log.h"
#include "raymath.h"

ok::opt_t<lib::Vect>
lib::tesselate::lineSegmentsIntersection(Vect p1, Vect p2, Vect p3, Vect p4,
                                         bool allowOnLine) noexcept
{
    Vect out = Vect::zero();

    float d = (p2.x - p1.x) * (p4.y - p3.y) - (p2.y - p1.y) * (p4.x - p3.x);

    if (d == 0.0f) {
        return ok::nullopt;
    }

    float u =
        ((p3.x - p1.x) * (p4.y - p3.y) - (p3.y - p1.y) * (p4.x - p3.x)) / d;
    float v =
        ((p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x)) / d;

    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f) {
        return ok::nullopt;
    }

    if (!allowOnLine && (u == 0 || u == 1 || v == 0 || v == 1)) {
        return ok::nullopt;
    }

    out.x = p1.x + u * (p2.x - p1.x);
    out.y = p1.y + u * (p2.y - p1.y);

    return out;
}

lib::Rect lib::tesselate::aabbFromPath(ok::slice_t<const Vect> path) noexcept
{
    float minX = 0;
    float maxX = 0;
    float minY = 0;
    float maxY = 0;
    minX = maxX = path[0].x;
    minY = maxY = path[0].y;
    ok_foreach(const Vect &v, path)
    {
        if (v.x < minX) {
            minX = v.x;
        }
        if (v.x > maxX) {
            maxX = v.x;
        }
        if (v.y < minY) {
            minY = v.y;
        }
        if (v.y > maxY) {
            maxY = v.y;
        }
    }
    return ::Rectangle{
        .x = minX,
        .y = minY,
        .width = maxX - minX,
        .height = maxY - minY,
    };
}

bool lib::tesselate::isPointInsidePolygon(ok::slice_t<const Vect> polygon,
                                          Vect point) noexcept
{
    Rect aabb = aabbFromPath(polygon);
    // Cast a ray from point to positive x
    Segment ray = Segment(point, ::Vector2{.x = aabb.max().x, .y = point.y});
    // Count intersections with path
    u64 intersections = 0;
    for (u64 i = 0; i < polygon.size(); ++i) {
        Vector2 p1 = polygon[i];
        Vector2 p2 = polygon[(i + 1) % polygon.size()];
        Vector2 unused;
        if (auto unused = lineSegmentsIntersection(p1, p2, ray.a, ray.b)) {
            ++intersections;
        }
    }

    return intersections % 2 == 1;
}

bool lib::tesselate::isPolygonClockwise(
    ok::slice_t<const lib::Vect> polygon) noexcept
{
    assert(polygon.size() >= 3);
    float sum = 0;
    for (u64 i = 0; i < polygon.size(); ++i) {
        Vector2 p1 = polygon[i];
        Vector2 p2 = polygon[(i + 1) % polygon.size()];
        sum += (p2.x - p1.x) * (p2.y + p1.y);
    }
    return sum >= 0;
}

bool lib::tesselate::isTriangleClockwise(Triangle tri) noexcept
{
    lib::Vect tris[3] = {tri.a, tri.b, tri.c};
    return isPolygonClockwise(tris);
}

auto lib::tesselate::certifyWindingOrder(Triangle tri,
                                         bool clockwise) noexcept -> Triangle
{
    if (isTriangleClockwise(tri) != clockwise) {
        // invert winding order of triangle
        Vector2 cache = tri.b;
        tri.b = tri.c;
        tri.c = cache;
    }
    return tri;
}

void lib::tesselate::certifyWindingOrder(ok::slice_t<Triangle> tris,
                                         bool clockwise) noexcept
{
    for (int i = 0; i < tris.size(); ++i) {
        tris[i] = certifyWindingOrder(tris[i], clockwise);
    }
}
auto lib::tesselate::triangulate(ok::slice_t<const Vect> pathSource,
                                 int subsequentCalls)
    -> ok::opt_t<std::vector<Triangle>>
{
    assert(pathSource.size() >= 3);

    // Copy path source over to path
    std::vector<Vect> path;
    ok_foreach(const Vect &vert, pathSource) { path.push_back(vert); }

    Segment segment = Segment(path[1], path[path.size() - 1]);

    bool segmentValid = true;

    // Does the center point of the segment lie outside the polygon?
    if (!isPointInsidePolygon(path, segment.center())) {
        segmentValid = false;
    }

    if (segmentValid) {
        // Does the segment [1, L-1] cut through the shape at any point?
        Vector2 center = segment.center();
        for (u64 i = 0; i < path.size(); ++i) {
            Vector2 p1 = path[i];
            Vector2 p2 = path[(i + 1) % path.size()];
            Vector2 unused;
            if (auto unused = lineSegmentsIntersection(p1, p2, segment.a,
                                                       segment.b, false)) {
                segmentValid = false;
                break;
            }
        }
    }

    if (!segmentValid) {
        // Try again from the next vertex

        if (subsequentCalls > path.size()) {
            // Rotated fully the vertices and never found a break. Something bad
            // happened :'(
            LN_ERROR("Cannot triangulate path. Rotated too many times "
                     "subsequently with no improvements.");
            return ok::nullopt;
        }

        rotatePathClockwise(path);
        return triangulate(path, subsequentCalls + 1);
    }

    // Ok! the segment being valid, let's extract that triangle from the path
    // and carry on.
    Triangle firstTri =
        Triangle{.a = path[0], .b = path[1], .c = path[path.size() - 1]};
    path.erase(path.begin());
    if (path.size() < 3) {
        return std::vector<Triangle>{firstTri};
    } else {
        // still have more than 4 vertices, keep going deeper
        std::vector<Triangle> result{firstTri};
        if (auto nextResult = triangulate(path)) {
            std::copy(nextResult.value().begin(), nextResult.value().end(),
                      std::back_inserter(result));
        } else {
            return ok::nullopt;
        }
        return result;
    }
}

void lib::tesselate::addVertexToMesh(std::vector<::Vector3> &vertices,
                                     std::vector<u16> &indices,
                                     Vector3 vertex) noexcept
{
    for (int i = 0; i < vertices.size(); ++i) {
        if (Vector3LengthSqr(Vector3Subtract(vertices[i], vertex)) < 0.001f) {
            indices.push_back(i);
            return;
        }
    }
    vertices.push_back(vertex);
    indices.push_back(vertices.size() - 1);
}

void lib::tesselate::addTriangleToMesh(std::vector<Vector3> &vertices,
                                       std::vector<u16> &indices, Triangle tri,
                                       float z, bool clockwise) noexcept
{
    tri = certifyWindingOrder(
        tri, clockwise); // make triangle clockwise (or counterclockwise)
    // Add the three vertices, in order.
    addVertexToMesh(vertices, indices, Vector3{tri.a.x, tri.a.y, z});
    addVertexToMesh(vertices, indices, Vector3{tri.b.x, tri.b.y, z});
    addVertexToMesh(vertices, indices, Vector3{tri.c.x, tri.c.y, z});
}

void lib::tesselate::addTrianglesToMesh(std::vector<Vector3> &vertices,
                                        std::vector<u16> &indices,
                                        ok::slice_t<const Triangle> tris,
                                        float z, bool clockwise) noexcept
{
    ok_foreach(const Triangle &tri, tris)
    {
        addTriangleToMesh(vertices, indices, tri, z, clockwise);
    }
}
