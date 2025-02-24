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
using fmt::println;

constexpr Vect render_size = {800, 600};
constexpr size_t fps = 60;

int main()
{
    using namespace werm;
    ln::init();
    InitWindow(render_size.x, render_size.y, "Squinchwerms");
    SetTargetFPS(fps);

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
    constexpr auto square = Rect::unitSquare().scaledBy(100.f).at({50.f, 50.f});
    lib::PolyShape &playerShape =
        game.physics.polyShapes
            .make(playerBody, lib::PolyShape::SquareOptions{square})
            .release();

    game.physics.space.add(playerBody);

    while (!WindowShouldClose()) {
        // game.physics.space.step(1.0f / 60.0f);

        // draw
        BeginDrawing();
        ClearBackground(WHITE);

        println("{}", playerBody.position());
        println("{}", playerShape.asShape().getBoundingBox());
        BeginMode2D(Camera2D{
            .offset = render_size / 2,
            .target = playerBody.position(),
            .zoom = 1,
        });

        DrawRectanglePro(playerShape.asShape().getBoundingBox(),
                         playerBody.position(), 0.f, RED);

        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    cpSpaceDestroy(&game.physics.space);
    game.meshes.forEach([](auto item) { UnloadMesh(item.self); });

    levelArena.destroy();

    return 0;
}
