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

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode2D(camera);
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    werm::clear_level();
    // WARN: all items allocated in level allocator are now invalid
    return 0;
}
