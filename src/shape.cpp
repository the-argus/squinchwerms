#include "shape.h"
#include "body.h"
#include <cstring>

namespace lib {
void Shape::_add_to_space(Space &_space)
{
    cpSpaceAddShape(reinterpret_cast<cpSpace *>(&_space), this);
    space = reinterpret_cast<cpSpace *>(&_space);
}

float Shape::friction() { return cpShapeGetFriction(this); }

void Shape::setFriction(float friction) { cpShapeSetFriction(this, friction); };

bool Shape::sensor() { return cpShapeGetSensor(this); }

void Shape::setSensor(bool is_sensor) { cpShapeSetSensor(this, is_sensor); }

float Shape::density() { return cpShapeGetDensity(this); }

void Shape::setDensity(float density) { cpShapeSetDensity(this, density); }

cpShapeFilter Shape::filter() { return cpShapeGetFilter(this); }

void Shape::setFilter(cpShapeFilter filter) { cpShapeSetFilter(this, filter); }

float Shape::elasticity() { return cpShapeGetElasticity(this); }

void Shape::setElasticity(float elasticity)
{
    cpShapeSetElasticity(this, elasticity);
}

cpDataPointer Shape::userData() { return cpShapeGetUserData(this); }

void Shape::setUserData(cpDataPointer pointer)
{
    cpShapeSetUserData(this, pointer);
}

Vect Shape::surfaceVelocity() { return cpShapeGetSurfaceVelocity(this); }

void Shape::setSurfaceVelocity(Vect velocity)
{
    cpShapeSetSurfaceVelocity(this, velocity);
}

cpCollisionType Shape::collisionType() { return cpShapeGetCollisionType(this); }

void Shape::setCollisionType(cpCollisionType type)
{
    cpShapeSetCollisionType(this, type);
}

void Shape::free() { cpShapeFree(this); }

Body &Shape::body() { return *static_cast<Body *>(cpShapeGetBody(this)); }

void Shape::removeFromSpace()
{
    if (space != nullptr)
        cpSpaceRemoveShape(space, this);
}

Rect Shape::getBoundingBox() { return cpShapeGetBB(this); }

int PolyShape::count() const { return cpPolyShapeGetCount(&shape); }
float PolyShape::radius() const { return cpPolyShapeGetRadius(&shape); }
Vect PolyShape::vertex(int index) const
{
    return cpPolyShapeGetVert(&shape, index);
}

PolyShape::PolyShape(lib::Body &body, const DefaultOptions &options)
{
    std::memset(static_cast<cpPolyShape *>(this), 0, sizeof(cpPolyShape));
    static_assert(
        sizeof(cpVect) == sizeof(lib::Vect),
        "Make sure lib::Vect and cpVect are laid out identically in memory");
    cpPolyShapeInitRaw(this, &body, static_cast<int>(options.vertices.size()),
                       reinterpret_cast<cpVect *>(options.vertices.data()),
                       options.radius);
}

PolyShape::PolyShape(lib::Body &body, const SquareOptions &options)
{
    std::memset(static_cast<cpPolyShape *>(this), 0, sizeof(cpPolyShape));
    cpBoxShapeInit2(this, &body, options.bounding, options.radius);
}

Vect SegmentShape::a() const { return cpSegmentShapeGetA(&shape); }
Vect SegmentShape::b() const { return cpSegmentShapeGetB(&shape); }
Vect SegmentShape::normal() const { return cpSegmentShapeGetNormal(&shape); }

SegmentShape &SegmentShape::setNeighbors(Vect prev, Vect next)
{
    cpSegmentShapeSetNeighbors(&shape, prev, next);
    return *this;
}

SegmentShape::SegmentShape(lib::Body &body, const Options &options) noexcept
{
    std::memset(static_cast<cpSegmentShape *>(this), 0, sizeof(cpSegmentShape));
    // TODO: figure out why reinterpret_cast is necessary here, Body
    // publicly inherits from cpBody?
    cpSegmentShapeInit(this, reinterpret_cast<cpBody *>(&body), options.a,
                       options.b, options.radius);
}
} // namespace lib
