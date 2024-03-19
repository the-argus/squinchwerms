#include "level.h"
#include "natural_log/natural_log.h"
#include "physics.h"
#include "vect.h"
#include <allo.h>
#include <cstddef>
#include <raylib.h>
#include <ziglike/zigstdint.h>

using namespace lib;

constexpr Vect render_size = {800, 600};
constexpr size_t fps = 60;

static Camera2D camera;

int main()
{
    ln::init();
    InitWindow(render_size.x, render_size.y, "Squinchwerms");
    SetTargetFPS(fps);

    camera = {
        .offset = render_size / 2,
        .target = {0, 0},
        .zoom = 1,
    };

    // initialize level allocators
    if (!werm::init_level().okay()) {
        LN_FATAL("Unable to initialize level");
        std::abort();
    }

    auto physics_res = werm::PhysicsSystem::make(werm::level_allocator());
    if (!physics_res.okay()) {
        LN_FATAL("Unable to initialize physics");
		// no need to clear level allocator, this is an unrecoverable error
        std::abort();
    }
    werm::PhysicsSystem &physics = physics_res.release();

    werm::BodyRef begin = physics
                              .create_body({.type = lib::Body::Type::DYNAMIC,
                                            .mass = 10,
                                            .moment = INFINITY})
                              .value();
    begin->set_position({0, 100});
    werm::BodyRef end = physics
                            .create_body({.type = lib::Body::Type::DYNAMIC,
                                          .mass = 10,
                                          .moment = INFINITY})
                            .value();
    end->set_position({100, 100});

    // spring exists for the whole level so just put it on the level stack
    werm::DampedSpringRef spring =
        physics
            .connect_with_damped_spring(
                werm::level_allocator(), begin, end,
                {.length = 10, .stiffness = 1, .damping = 1})
            .value();

    lib::Rect floor = {{0, 0}, {800, 10}};
    physics.create_square(physics.static_body(),
                          {.bounding = floor, .radius = 1});

    Texture end_tex = LoadTexture("assets/img/werm/end.png");
    Texture begin_tex = LoadTexture("assets/img/werm/head.png");

    while (!WindowShouldClose()) {
        // clear any frame-specific allocations
        werm::clear_frame();
        // update
        {
            physics.update();
        }

        // draw
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode2D(camera);
        DrawTextureV(end_tex, end->position(), WHITE);
        DrawTextureV(begin_tex, begin->position(), WHITE);
        DrawRectangleRec(floor, WHITE);
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    werm::clear_level();
    // WARN: all items allocated in level allocator are now invalid
    return 0;
}
