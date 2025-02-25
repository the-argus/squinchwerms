#include "debug_draw.h"
#include "natural_log/natural_log.h"
#include "raymath.h"
#include "shape.h"
#include "space.h"
#include "vect.h"
#include "tesselator.h"
#include <okay/short_arithmetic_types.h>
#include <raylib.h>
using namespace lib;
struct Vert
{
    Vect pos;
    Vect uv;
    float r;
    Color fill, outline;
};

static float lineScale = 1.0f;

// Meh, just max out 16 bit index size.
#define VERTEX_MAX (64 * 1024)
#define INDEX_MAX (4 * VERTEX_MAX)

using Index = u16;

static Vector3 positions[VERTEX_MAX];
static Vect uvs[VERTEX_MAX];
static Color colors[VERTEX_MAX];
static Index Indexes[INDEX_MAX];

static u64 VertexCount;
static u64 IndexCount;

static void writeVert(const Vert &vertex, Index i)
{
    positions[i] = Vector3{.x = vertex.pos.x, .y = vertex.pos.y};
    uvs[i] = vertex.uv;
    colors[i] = vertex.fill;
}

/// Allocates some space in the buffers and initializes indices properly
/// Returns the starting index in the buffers where you may write
static u64 pushVertices(size_t vcount, const Index *index_src, size_t icount)
{
    if (!(VertexCount + vcount <= VERTEX_MAX &&
          IndexCount + icount <= INDEX_MAX)) {
        LN_FATAL("geometry buffer full");
        std::abort();
    }

    const u64 vertex_dst = VertexCount;
    u64 base = VertexCount;
    VertexCount += vcount;

    Index *index_dst = Indexes + IndexCount;
    for (size_t i = 0; i < icount; i++)
        index_dst[i] = index_src[i] + (Index)base;
    IndexCount += icount;

    return vertex_dst;
}

Mesh *lib::genMeshFromVertices(ok::slice_t<cpVect> vertSlice, float radius,
                               Color fill) noexcept
{
    IndexCount = 0;
    VertexCount = 0;
    cpVect *verts = vertSlice.data();
    u64 count = vertSlice.size();
#define MAX_POLY_VERTEXES 64
// Fill needs (count - 2) triangles.
// Outline needs 4*count triangles.
#define MAX_POLY_INDEXES (3 * (5 * MAX_POLY_VERTEXES - 2))
    u16 indexes[MAX_POLY_INDEXES];
    // Polygon fill triangles.
    for (int i = 0; i < count - 2; i++) {
        indexes[3 * i + 0] = 0;
        indexes[3 * i + 1] = 4 * (i + 1);
        indexes[3 * i + 2] = 4 * (i + 2);
    }

    // Polygon outline triangles.
    Index *cursor = indexes + 3 * (count - 2);
    for (int i0 = 0; i0 < count; i0++) {
        int i1 = (i0 + 1) % count;
        cursor[12 * i0 + 0] = 4 * i0 + 0;
        cursor[12 * i0 + 1] = 4 * i0 + 1;
        cursor[12 * i0 + 2] = 4 * i0 + 2;
        cursor[12 * i0 + 3] = 4 * i0 + 0;
        cursor[12 * i0 + 4] = 4 * i0 + 2;
        cursor[12 * i0 + 5] = 4 * i0 + 3;
        cursor[12 * i0 + 6] = 4 * i0 + 0;
        cursor[12 * i0 + 7] = 4 * i0 + 3;
        cursor[12 * i0 + 8] = 4 * i1 + 0;
        cursor[12 * i0 + 9] = 4 * i0 + 3;
        cursor[12 * i0 + 10] = 4 * i1 + 0;
        cursor[12 * i0 + 11] = 4 * i1 + 1;
    }

    float inset = (float)-cpfmax(0, 2 * lineScale - radius);
    float outset = (float)radius + lineScale;
    float r = outset - inset;

    const u64 vertexes = pushVertices(4 * count, indexes, 3 * (5 * count - 2));
    for (int i = 0; i < count; i++) {
        cpVect v0 = verts[i];
        cpVect v_prev = verts[(i + (count - 1)) % count];
        cpVect v_next = verts[(i + (count + 1)) % count];

        cpVect n1 = cpvnormalize(cpvrperp(cpvsub(v0, v_prev)));
        cpVect n2 = cpvnormalize(cpvrperp(cpvsub(v_next, v0)));
        cpVect of = cpvmult(cpvadd(n1, n2), 1.0 / (cpvdot(n1, n2) + 1.0f));
        cpVect v = cpvadd(v0, cpvmult(of, inset));

        writeVert(Vert{.pos = Vect{(float)v.x, (float)v.y},
                       .uv = Vect{0.0f, 0.0f},
                       .r = 0.0f,
                       .fill = fill},
                  vertexes + (4 * i + 0));
        writeVert(Vert{.pos = Vect{(float)v.x, (float)v.y},
                       .uv = {(float)n1.x, (float)n1.y},
                       .r = r,
                       .fill = fill},
                  vertexes + (4 * i + 1));
        writeVert(Vert{.pos = {(float)v.x, (float)v.y},
                       .uv = {(float)of.x, (float)of.y},
                       .r = r,
                       .fill = fill},
                  vertexes + (4 * i + 2));
        writeVert(Vert{.pos = {(float)v.x, (float)v.y},
                       .uv = {(float)n2.x, (float)n2.y},
                       .r = r,
                       .fill = fill},
                  vertexes + (4 * i + 3));
    }

    Mesh *out = (Mesh *)malloc(sizeof(Mesh));
    *out = Mesh{
        .vertices = (float *)malloc(sizeof(Vector3) * VertexCount),
        .texcoords = (float *)malloc(sizeof(Vect) * VertexCount),
        .colors = (unsigned char *)malloc(sizeof(Color) * VertexCount),
        .indices =
            (unsigned short *)malloc(sizeof(unsigned short) * IndexCount),
    };

    std::memcpy(out->vertices, positions, sizeof(Vector3) * VertexCount);
    std::memcpy(out->texcoords, uvs, sizeof(Vect) * VertexCount);
    std::memcpy(out->colors, colors, sizeof(Color) * VertexCount);
    std::memcpy(out->indices, Indexes, sizeof(unsigned short) * VertexCount);

    UploadMesh(out, false);

    return out;
}

static void debugDrawSegmentShape(cpSegmentShape *segmentShape,
                                  Color color) noexcept
{
    Vect pos = cpBodyGetPosition(segmentShape->shape.body);
    Vect start = Vect(segmentShape->a) + pos;
    Vect end = Vect(segmentShape->b) + pos;
	fmt::println("drawing line from {} to {} in local space", Vect(segmentShape->a), Vect(segmentShape->b));
	fmt::println("\tBUT from {} to {} in global space", start, end);
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
