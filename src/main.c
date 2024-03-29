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

    if (!werm::init_physics(werm::level_allocator()).okay()) {
        LN_FATAL("Unable to initialize physics");
        std::abort();
    }

    werm::BodyRef begin =
        werm::create_body(
            {.type = lib::Body::Type::DYNAMIC, .mass = 10, .moment = INFINITY})
            .value();
    begin->set_position({0, 100});
    werm::BodyRef end =
        werm::create_body(
            {.type = lib::Body::Type::DYNAMIC, .mass = 10, .moment = INFINITY})
            .value();
    end->set_position({100, 100});

    // spring exists for the whole level so just put it on the level stack
    werm::DampedSpringRef spring =
        werm::connect_with_damped_spring(
            werm::level_allocator(), begin, end,
            {.length = 10, .stiffness = 1, .damping = 1})
            .value();

    lib::Rect floor = {{0, 0}, {800, 10}};
    werm::create_square(werm::static_body(), {.bounding = floor, .radius = 1});

    Texture end_tex = LoadTexture("assets/img/werm/end.png");
    Texture begin_tex = LoadTexture("assets/img/werm/head.png");

    while (!WindowShouldClose()) {
        werm::update_physics();
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
