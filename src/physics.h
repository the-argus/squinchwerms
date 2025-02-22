#pragma once
#include "body.h"
#include "linked_pool.h"
#include "space.h"

namespace werm {

struct Physics
{
    LinkedPool<lib::Body> bodies;
    LinkedPool<lib::PolyShape> polyShapes;
    LinkedPool<lib::SegmentShape> segmentShapes;
    lib::Space space;
};
} // namespace werm
