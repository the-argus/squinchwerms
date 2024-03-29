#include "level.h"
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int x;
    int y;
} Vector2I;

const Vector2I render_size = {800, 600};
const Vector2 half_render_size = {400, 300};
const int fps = 60;

static Camera2D camera;

int main()
{
    InitWindow(render_size.x, render_size.y, "Squinchwerms");
    SetTargetFPS(fps);

    camera = (Camera2D){
        .offset = (Vector2){half_render_size.x, half_render_size.y},
        .target = {0, 0},
        .zoom = 1,
    };

    // initialize level allocators
    if (!init_level()) {
        printf("Unable to initialize level\n");
        abort();
    }

    // werm::PhysicsSystem &physics = physics_res.release();

    // werm::BodyRef begin = physics
    //                           .create_body({.type = lib::Body::Type::DYNAMIC,
    //                                         .mass = 10,
    //                                         .moment = INFINITY})
    //                           .value();
    // begin->set_position({0, 100});
    // werm::BodyRef end = physics
    //                         .create_body({.type = lib::Body::Type::DYNAMIC,
    //                                       .mass = 10,
    //                                       .moment = INFINITY})
    //                         .value();
    // end->set_position({100, 100});

    // // spring exists for the whole level so just put it on the level stack
    // werm::DampedSpringRef spring =
    //     physics
    //         .connect_with_damped_spring(
    //             werm::level_allocator(), begin, end,
    //             {.length = 10, .stiffness = 1, .damping = 1})
    //         .value();

    // lib::Rect floor = {{0, 0}, {800, 10}};
    // physics.create_square(physics.static_body(),
    //                       {.bounding = floor, .radius = 1});

    // Texture end_tex = LoadTexture("assets/img/werm/end.png");
    // Texture begin_tex = LoadTexture("assets/img/werm/head.png");

    while (!WindowShouldClose()) {
        // clear any frame-specific allocations
        clear_frame();
        // update
        {
            // physics.update();
        }

        // draw
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode2D(camera);
        // DrawTextureV(end_tex, end->position(), WHITE);
        //  DrawTextureV(begin_tex, begin->position(), WHITE);
        // DrawRectangleRec(floor, WHITE);
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    clear_level();
    // WARN: all items allocated in level allocator are now invalid
    return 0;
}
