#include "debug_draw.h"
#include "natural_log/natural_log.h"
#include "raymath.h"
#include "shape.h"
#include "space.h"
#include "tesselator.h"
#include "vect.h"
#include <array>
#include <okay/short_arithmetic_types.h>
#include <raylib.h>
using namespace lib;

Mesh *lib::genMeshFromVertices(ok::slice_t<const cpVect> vertSlice,
                               float radius, Color fill) noexcept
{
    auto path_polygon = ok::raw_slice(
        *reinterpret_cast<const Vect *>(vertSlice.data()), vertSlice.size());

    auto result = lib::tesselate::triangulate(path_polygon);
    if (!result) {
        LN_ERROR("failed to tesselate shape into mesh for debug drawing");
        return nullptr;
    }

    auto &triangles = result.value();

    std::vector<Vector3> positions;
    std::vector<u16> indices;
    lib::tesselate::addTrianglesToMesh(positions, indices, triangles, 0.0f,
                                       true);

    Mesh *out = (Mesh *)malloc(sizeof(Mesh));
    *out = Mesh{
        .vertexCount = int(positions.size()),
        .triangleCount = int(triangles.size()),
        .vertices = (float *)malloc(sizeof(Vector3) * positions.size()),
        .texcoords = (float *)calloc(positions.size(), sizeof(Vector2)),
        .colors = (unsigned char *)calloc(positions.size(), sizeof(Color)),
        .indices = (u16 *)malloc(sizeof(u16) * indices.size()),
    };

    for (u64 i = 0; i < positions.size(); ++i) {
        ((Color *)out->colors)[i] = fill;
    }

    std::memcpy(out->vertices, positions.data(),
                sizeof(Vector3) * positions.size());
    std::memcpy(out->indices, indices.data(), sizeof(u16) * indices.size());

    UploadMesh(out, false);

    return out;
}

static void debugDrawSegmentShape(cpSegmentShape *segmentShape,
                                  Color color) noexcept
{
    Vect pos = cpBodyGetPosition(segmentShape->shape.body);
    Vect start = Vect(segmentShape->a) + pos;
    Vect end = Vect(segmentShape->b) + pos;
    DrawLine(start.x, start.y, end.x, end.y, color);
}

static void debugDrawPolyShape(cpPolyShape *polyShape,
                               Material material) noexcept
{
    if (!polyShape->shape.userData) {
        LN_ERROR("missing user data from polygon shape, no mesh to draw");
        return;
    }

    Mesh *mesh = (Mesh *)polyShape->shape.userData;
    auto pos = cpBodyGetPosition(polyShape->shape.body);
    DrawMesh(*mesh, material, MatrixTranslate(pos.x, pos.y, 0));

    // for (u64 i = 0; i < mesh->triangleCount; ++i) {
    //     auto &indexStart = *(std::array<u16, 3> *)(mesh->indices + (i * 3 *
    //     3)); Vector3 *p1 = (Vector3 *)(mesh->vertices + (indexStart[0] * 3));
    //     Vector3 *p2 = (Vector3 *)(mesh->vertices + (indexStart[1] * 3));
    //     Vector3 *p3 = (Vector3 *)(mesh->vertices + (indexStart[2] * 3));
    //     DrawTriangle({p1->x + pos.x, p1->y + pos.y},
    //                  {p2->x + pos.x, p2->y + pos.y},
    //                  {p3->x + pos.x, p3->y + pos.y}, RED);
    // }
}

void lib::debugDrawPhysics(lib::Space &space, Material polygonMaterial) noexcept
{
    for (size_t i = 0; i < space.dynamicBodies->num; ++i) {
        cpBody *body = (cpBody *)space.dynamicBodies->arr[i];
        cpShape *shapeIter = body->shapeList;
        while (shapeIter) {
            switch (shapeIter->klass->type) {
            case CP_POLY_SHAPE:
                debugDrawPolyShape((cpPolyShape *)shapeIter, polygonMaterial);
                break;
            case CP_SEGMENT_SHAPE:
                debugDrawSegmentShape((cpSegmentShape *)shapeIter, RED);
                break;
            default:
                LN_INFO_FMT("unknown shape type {}",
                            int(shapeIter->klass->type));
                break;
            }
            shapeIter = shapeIter->next;
        }
    }
}
