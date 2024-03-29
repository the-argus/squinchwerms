#include "intshorthand.h"
#include "level.h"
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const Vector2 render_size = {800, 600};
const size_t fps = 60;

static Camera2D camera;

int main()
{
    InitWindow(render_size.x, render_size.y, "Squinchwerms");
    SetTargetFPS(fps);

    camera = (Camera2D){
        .offset = (Vector2){render_size.x / 2, render_size.y / 2},
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

    Texture end_tex = LoadTexture("assets/img/werm/end.png");
    Texture begin_tex = LoadTexture("assets/img/werm/head.png");

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
