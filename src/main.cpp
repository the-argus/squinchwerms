#include <raylib.h>

#include "debug_draw.h"
#include "game.h"
#include "natural_log/natural_log.h"
#include "vect.h"
#include "json/json_space.h"
#include <cstddef>
#include <imgui.h>
#include <okay/allocators/arena.h>
#include <okay/allocators/c_allocator.h>
#include <okay/short_arithmetic_types.h>
#include <raylib.h>
#include <rlImGui.h>

using namespace lib;

constexpr Vect render_size = {800, 600};
constexpr size_t fps = 60;

enum class MenuAction
{
    EnterGame,
    None,
    ExitGame,
};

static MenuAction runMainMenu() noexcept;

int main()
{
    using namespace werm;
    ln::init();
    InitWindow(render_size.x, render_size.y, "Squinchwerms");
    SetTargetFPS(fps);
    rlImGuiSetup(true);

    ok::c_allocator_t backing;
    ok::arena_t levelArena(
        backing.allocate({.num_bytes = 1024 * 1024}).release().as_bytes(),
        backing);

    static Game game{
        .backingHeapAllocator = backing,
        .levelArena = levelArena,
        .meshes = {.backingAllocator = levelArena},
        .physics =
            {
                .bodies = {.backingAllocator = levelArena},
                .polyShapes = {.backingAllocator = levelArena},
                .segmentShapes = {.backingAllocator = levelArena},
                .space = lib::Space{lib::zeroed_tag{}},
            },
    };

    bool good = deserializeSpace(
        game.physics.space, "example.sqw",
        lib::PhysicsAllocators{
            .bodyAllocator =
                [] {
                    return &game.physics.bodies.make(lib::zeroed_tag{})
                                .release();
                },
            .polyShapeAllocator =
                [] {
                    return &game.physics.polyShapes.make(lib::zeroed_tag{})
                                .release();
                },
            .segmentShapeAllocator =
                [] {
                    return &game.physics.segmentShapes.make(lib::zeroed_tag{})
                                .release();
                },
            .vertexBufScratch = backing,
        });

    if (!good) {
        exit(1);
    }

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

    game.physics.space.setGravity({0, -10.f});

    bool inGame = false;
    bool exitWindow = false;

    Camera2D camera = {
        .offset = render_size / 2,
        .target = lib::Vect::zero(),
        .zoom = 1.f,
    };

    Material def = LoadMaterialDefault();
	def.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;

    while (!WindowShouldClose() && !exitWindow) {
        if (inGame) {
            game.physics.space.step(1.0f / 60.0f);

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                Vect delta = GetMouseDelta();
                camera.target =
                    Vect(camera.target) + delta.negative() / camera.zoom;
            }

            camera.zoom += GetMouseWheelMove() * 0.1f;
            camera.zoom = ok::partial_clamp(camera.zoom, 0.1f, 3.f);
        }

        // draw
        BeginDrawing();

        if (inGame) {
            ClearBackground(WHITE);

            EndMode3D();

            // println("{}", playerBody.position());
            // println("{}", playerShape.asShape().getBoundingBox());
            BeginMode2D(camera);

            DrawRectangle(0, 0, 50, 50, BLACK);

            debugDrawPhysics(game.physics.space, def);

            EndMode2D();
        } else {
            switch (runMainMenu()) {
            case MenuAction::None:
                break;
            case MenuAction::ExitGame:
                exitWindow = true;
                break;
            case MenuAction::EnterGame:
                inGame = true;
                break;
            }
        }

        EndDrawing();
    }

    rlImGuiShutdown();

    CloseWindow();

    cpSpaceDestroy(&game.physics.space);
    game.meshes.forEach([](auto item) { UnloadMesh(item.self); });

    levelArena.destroy();

    return 0;
}

static MenuAction runMainMenu() noexcept
{
    ClearBackground(WHITE);
    rlImGuiBegin();

    auto action = MenuAction::None;
    auto noOtherButtonsPressed = [&] { return action == MenuAction::None; };

    using namespace ImGui;

    BeginGroup();
    {
        if (Button("Squinch")) {
            action = MenuAction::EnterGame;
        }

        if (Button("Exit Game") && noOtherButtonsPressed()) {
            action = MenuAction::ExitGame;
        }
    }
    EndGroup();

    rlImGuiEnd();

    return action;
}
