#pragma once
#include "rect.h"
#include "vect.h"
#include <chipmunk/chipmunk.h>
#include <chipmunk/chipmunk_structs.h>
#include <ziglike/slice.h>

namespace lib {

class Body;
class Space;

class Shape : public ::cpShape
{
  public:
    // youre not inteded to make one of these, its abstract
    Shape() = delete;

    float friction();
    void set_friction(float friction);
    bool sensor();
    void set_sensor(bool is_sensor);
    float density();
    void set_density(float density);
    cpShapeFilter filter();
    void set_filter(cpShapeFilter filter);
    float elasticity();
    void set_elasticity(float elasticity);
    cpDataPointer user_data();
    void set_user_data(cpDataPointer pointer);
    Vect surface_velocity();
    void set_surface_velocity(Vect velocity);
    cpCollisionType collision_type();
    void set_collision_type(cpCollisionType type);

    Rect get_bounding_box();

    // not a destructor becasue we'd like to be able to cast cpShapes to this
    // wtihout invoking RAII
    void free();

    Body *body();

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

    void remove_from_space();

  private:
    void _add_to_space(Space *space);
    friend Space;
};

class PolyShape : public ::cpPolyShape
{
  public:
    // cannot be default constructed because initializing a shape requires a
    // body
    PolyShape() = delete;

    struct default_options_t
    {
        zl::slice<Vect> vertices;
        float radius;
    };

    PolyShape(Body &body, const default_options_t &options);

    struct square_options_t
    {
        Rect bounding;
        float radius;
    };

    PolyShape(Body &body, const square_options_t &options);

    [[nodiscard]] int count() const;
    [[nodiscard]] float radius() const;
    [[nodiscard]] Vect vertex(int index) const;
    inline Shape *parent_cast() { return static_cast<Shape *>(&shape); }
};

class SegmentShape : public ::cpSegmentShape
{
  public:
    SegmentShape() = delete;

    struct options_t
    {
        Vect a;
        Vect b;
        float radius;
    };

    SegmentShape(Body &body, const options_t &options) noexcept;

    [[nodiscard]] Vect a() const;
    [[nodiscard]] Vect b() const;
    [[nodiscard]] Vect normal() const;

    void set_neighbors(Vect prev, Vect next);
    inline Shape *parent_cast() { return static_cast<Shape *>(&shape); }
};

} // namespace lib
