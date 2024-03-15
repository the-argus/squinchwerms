#include "level.h"
#include "space.h"
#include "vect.h"
#include <allo.h>
#include <cstddef>
#include <raylib.h>

using namespace lib;

constexpr Vect render_size = {800, 600};
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

    // initialize level allocators
    if (!werm::init_level().okay()) {
        std::abort();
    }

    auto ref = werm::level_allocator();
    cpSpace &space = allo::construct_one<lib::Space>(ref).release();

    // unfortunately, chipmunk allocates other resources which the allocator
    // doesn't know about, so we have to hook in the cpSpaceDestroy function
    auto res = werm::level_allocator().register_destruction_callback(
        [](void *data) { ((lib::Space *)data)->lib::Space::~Space(); }, &space);

    if (!res.okay())
        std::abort();

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
