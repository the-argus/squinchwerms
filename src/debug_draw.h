#pragma once

#include <chipmunk/chipmunk.h>
#include <raylib.h>
#include "space.h"

namespace lib
{
void debugDrawPhysics(lib::Space &space, Material polygonMaterial) noexcept;

Mesh* genMeshFromVertices(ok::slice_t<cpVect> vertices, float radius, Color fill) noexcept;
}
