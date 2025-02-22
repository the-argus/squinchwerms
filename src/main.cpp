#include <raylib.h>

#include "natural_log/natural_log.h"
#include "physics.h"
#include "terrain.h"
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
    ok::arena_t physics_arena(
        backing.allocate({.num_bytes = 1024 * 1024}).release().as_bytes());
    ok::arena_t level_arena(
        backing.allocate({.num_bytes = 1024 * 1024}).release().as_bytes());

	auto& physics = physics_arena.make<PhysicsSystem>().release();

	auto terrain = level_arena.make<Terrain, amount_filled_tag>(Terrain::Coord{.x = 100, .y = 100}, 2).release();
    auto terrain_res = werm::Terrain::make_with(werm::level_allocator(),
                                                {.x = 100, .y = 100}, 2);
    if (!terrain_res) {
        LN_FATAL("Unable to initialize terrain");
        std::abort();
    }

    werm::Terrain &terrain = terrain_res.value();

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
        DrawRectangleRec(floor, BLACK);
        // terrain.draw();
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    werm::clear_level();
    // WARN: all items allocated in level allocator are now invalid
    return 0;
}
