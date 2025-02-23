#include <raylib.h>

#include "game.h"
#include "natural_log/natural_log.h"
#include "vect.h"
#include <cstddef>
#include <okay/allocators/arena.h>
#include <okay/allocators/c_allocator.h>
#include <okay/short_arithmetic_types.h>
#include <raylib.h>

using namespace lib;

constexpr Vect render_size = {800, 600};
constexpr size_t fps = 60;

static Camera2D camera;

int main()
{
    using namespace werm;
    ln::init();
    InitWindow(render_size.x, render_size.y, "Squinchwerms");
    SetTargetFPS(fps);

    camera = {
        .offset = render_size / 2,
        .target = {0, 0},
        .zoom = 1,
    };

    ok::c_allocator_t backing;
    ok::arena_t levelArena(
        backing.allocate({.num_bytes = 1024 * 1024}).release().as_bytes(),
        backing);

    Game game{
        .backingHeapAllocator = backing,
        .levelArena = levelArena,
        .meshes = {.backingAllocator = levelArena},
        .physics =
            {
                .bodies = {.backingAllocator = levelArena},
                .polyShapes = {.backingAllocator = levelArena},
                .segmentShapes = {.backingAllocator = levelArena},
                .space = lib::Space{},
            },
    };

    lib::Body &playerBody = game.physics.bodies
                                .make(lib::Body::BodyOptions{
                                    .type = lib::Body::Type::DYNAMIC,
                                    .mass = 1,
                                    .moment = 0.1f,
                                })
                                .release();
    lib::PolyShape &playerShape =
        game.physics.polyShapes
            .make(playerBody,
                  lib::PolyShape::SquareOptions{
                      Rect::unitSquare().at({0, 0}).scaledBy(1.f)})
            .release();

    game.physics.space.add(playerBody);

    while (!WindowShouldClose()) {
        game.physics.space.step(1.0f / 60.0f);

        // draw
        BeginDrawing();
        ClearBackground(WHITE);
		camera.offset = playerBody.position();
        BeginMode2D(camera);

        Rect(cpShapeGetBB(&playerShape.shape)).draw(RED);

        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    cpSpaceDestroy(&game.physics.space);
    game.meshes.forEach([](auto item) { UnloadMesh(item.self); });

    levelArena.destroy();

    return 0;
}
