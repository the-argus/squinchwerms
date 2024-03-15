#include "vect.h"
#include <cstddef>
#include <raylib.h>

using namespace lib;

constexpr vect render_size = {800, 600};
constexpr size_t fps = 60;

static Camera2D camera;

int main()
{
    InitWindow(render_size.x, render_size.y, "Squinchwerms");
    SetTargetFPS(fps);

    camera = {
        .offset = render_size / 2,
        .target = {0, 0},
        .zoom = 1,
    };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode2D(camera);
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
