#include "level.h"
#include "physics.h"
#include "rect.h"
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

    {
        physics_system_t *p = (physics_system_t *)STACK_ALLOC_ONE(
                                  level_allocator(), physics_system_t)
                                  .data;
        if (!physics_system_init(level_allocator(), p)) {
            printf("FATAL: unable to initialize physics system\n");
            abort();
        }
        physics_system_register_singleton(p);
    }

    cpBB floor = (cpBB){
        .b = 0,
        .l = -half_render_size.x,
        .r = half_render_size.x,
        .t = 20,
    };
    physics_system_create_square(
        physics_system_instance(),
        &(physics_square_shape_options_t){
            .body = physics_system_static_body(physics_system_instance()),
            .bounding = floor,
            .radius = 1.0f,
        });

    const float player_mass = 10;
    cpBody *player_body =
        physics_system_create_body(physics_system_instance(), player_mass);
    physics_system_create_square(
        physics_system_instance(),
        &(physics_square_shape_options_t){
            .body = player_body,
            .bounding = (cpBB){.b = 5, .t = -5, .l = -5, .r = 5},
            .radius = 1.0f,
        });

    // Texture end_tex = LoadTexture("assets/img/werm/end.png");
    Texture begin_tex = LoadTexture("assets/img/werm/head.png");

    while (!WindowShouldClose()) {
        // clear any frame-specific allocations
        clear_frame();
        // update
        {
            physics_system_update(physics_system_instance());
        }

        // draw
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode2D(camera);
        // DrawTextureV(end_tex, end->position(), WHITE);
        // cpVect pos = cpBodyGetPosition(begin);
        cpVect player_pos = cpBodyGetPosition(player_body);
        DrawTextureV(begin_tex, *(Vector2 *)&player_pos, WHITE);
        DrawRectangleRec(cpbb_to_raylib(floor), WHITE);
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    // NOTE: chipmunk doesnt use our allocators, so we have to clean up its shit
    // before we clear the allocator which holds out handle into those resources
    physics_system_free_chipmunk_extra_resources(physics_system_instance());
#ifndef NDEBUG
    physics_system_unregister_singleton();
#endif
    clear_level();
    // WARN: all items allocated in level allocator are now invalid
    return 0;
}
