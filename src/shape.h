#pragma once
#include "rect.h"
#include "vect.h"
#include <chipmunk/chipmunk.h>
#include <chipmunk/chipmunk_structs.h>
#include <okay/slice.h>

namespace lib {

class Body;
class Space;

class Shape : public ::cpShape
{
  public:
    // youre not inteded to make one of these, its abstract
    Shape() = delete;

    float friction();
    void setFriction(float friction);
    bool sensor();
    void setSensor(bool isSensor);
    float density();
    void setDensity(float density);
    cpShapeFilter filter();
    void setFilter(cpShapeFilter filter);
    float elasticity();
    void setElasticity(float elasticity);
    cpDataPointer userData();
    void setUserData(cpDataPointer pointer);
    Vect surfaceVelocity();
    void setSurfaceVelocity(Vect velocity);
    cpCollisionType collisionType();
    void setCollisionType(cpCollisionType type);

    Rect getBoundingBox();

    // not a destructor becasue we'd like to be able to cast cpShapes to this
    // wtihout invoking RAII
    void free();

    Body &body();

    // initialize separately

    /// Circle constructor
    // void Init(ShapeType type, const Body &body, float radius, Vect offset);
    // /// Segment constructor
    // void Init(ShapeType type, const Body &body, Vect a, Vect b,
    //           float radius);
    // /// Poly shape constructor
    // void Init(ShapeType type, const Body &body, Vect sides[],
    //           const int number_of_sides, cpTransform transform, float
    //           radius);
    // /// Poly shape constructor with default radius of 1
    // void Init(ShapeType type, const Body &body, Vect sides[],
    //           const int number_of_sides, cpTransform transform);
    // /// Poly shape which is a box
    // void Init(ShapeType type, const Body &body, float width, float height,
    //           float radius);
    // copy assignment op

    void removeFromSpace();

  private:
    void _add_to_space(Space &space);
    friend Space;
};

class PolyShape : public ::cpPolyShape
{
  public:
    // cannot be default constructed because initializing a shape requires a
    // body
    PolyShape() = delete;

    struct DefaultOptions
    {
        ok::slice_t<Vect> vertices;
        float radius;
    };

    PolyShape(Body &body, const DefaultOptions &options);

    struct SquareOptions
    {
        Rect bounding;
        float radius = 0.1f;
    };

    PolyShape(Body &body, const SquareOptions &options);

    [[nodiscard]] int count() const;
    [[nodiscard]] float radius() const;
    [[nodiscard]] Vect vertex(int index) const;
    constexpr Shape &parentCast() { return *static_cast<Shape *>(&shape); }
};

class SegmentShape : public ::cpSegmentShape
{
  public:
    SegmentShape() = delete;

    struct Options
    {
        Vect a;
        Vect b;
        float radius;
    };

    SegmentShape(Body &body, const Options &options) noexcept;

    [[nodiscard]] Vect a() const;
    [[nodiscard]] Vect b() const;
    [[nodiscard]] Vect normal() const;

    SegmentShape& setNeighbors(Vect prev, Vect next);
    constexpr Shape *parentCast() { return static_cast<Shape *>(&shape); }
};

} // namespace lib
