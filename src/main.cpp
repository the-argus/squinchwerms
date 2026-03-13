#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#define SDL_MAIN_USE_CALLBACKS 1 /* SDL defines main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>

#include <chrono>
#include <filesystem>
#include <optional>

#include "config.h"
#include "game_lib.h"

// i hate chrono
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using Duration =
    decltype(std::declval<TimePoint>() - std::declval<TimePoint>());

struct AppState
{
    GameLib gameLib;
    TimePoint lastHotreloadRecompileTime = std::chrono::system_clock::now();

    void reloadIfNeeded();
};

constexpr Duration recompilationTimeout = std::chrono::seconds(2);
constexpr float render_size[] = {800, 600};
constexpr size_t fps = 60;

[[nodiscard]] static std::optional<std::filesystem::file_time_type>
getMostRecentModifyTime(const char *sourceRootPath);
[[nodiscard]] static bool scanForSourceChanges(const char *sourceRootPath,
                                               const char *gameLibPath,
                                               TimePoint lastRecompilationTime);

// int main()
// {
//     ln::init();
//     InitWindow(render_size.x, render_size.y, "Squinchwerms");
//     SetTargetFPS(fps);
//     rlImGuiSetup(true);

//     ok::c_allocator_t backing;
//     ok::arena_t levelArena(
//         backing.allocate({.num_bytes = 1024 * 1024}).release().as_bytes(),
//         backing);

//     static Game game{
//         .backingHeapAllocator = backing,
//         .levelArena = levelArena,
//         .meshes = {.backingAllocator = levelArena},
//         .physics =
//             {
//                 .bodies = {.backingAllocator = levelArena},
//                 .polyShapes = {.backingAllocator = levelArena},
//                 .segmentShapes = {.backingAllocator = levelArena},
//                 .space = lib::Space{lib::zeroed_tag{}},
//             },
//     };

//     bool good = deserializeSpace(
//         game.physics.space, "example.sqw",
//         lib::PhysicsAllocators{
//             .bodyAllocator =
//                 [] {
//                     return &game.physics.bodies.make(lib::zeroed_tag{})
//                                 .release();
//                 },
//             .polyShapeAllocator =
//                 [] {
//                     return &game.physics.polyShapes.make(lib::zeroed_tag{})
//                                 .release();
//                 },
//             .segmentShapeAllocator =
//                 [] {
//                     return
//                     &game.physics.segmentShapes.make(lib::zeroed_tag{})
//                                 .release();
//                 },
//             .vertexBufScratch = backing,
//         });

//     if (!good) {
//         exit(1);
//     }

//     lib::Body &playerBody = game.physics.bodies
//                                 .make(lib::Body::BodyOptions{
//                                     .type = lib::Body::Type::DYNAMIC,
//                                     .mass = 1,
//                                     .moment = 0.1f,
//                                 })
//                                 .release();
//     constexpr auto square =
//     Rect::unitSquare().scaledBy(100.f).at({50.f, 50.f}); lib::PolyShape
//     &playerShape =
//         game.physics.polyShapes
//             .make(playerBody, lib::PolyShape::SquareOptions{square})
//             .release();

//     game.physics.space.add(playerBody);

//     game.physics.space.setGravity({0, -10.f});

//     bool inGame = false;
//     bool exitWindow = false;

//     Camera2D camera = {
//         .offset = render_size / 2,
//         .target = lib::Vect::zero(),
//         .zoom = 1.f,
//     };

//     Material def = LoadMaterialDefault();
//     def.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
//     def.maps[MATERIAL_MAP_DIFFUSE].texture =
//         LoadTexture("assets/terrain_textures/dirt_00.png");
//     def.shader = LoadShader("assets/shaders/passthrough.vert",
//                             "assets/shaders/biplanar_mapping.frag");

//     while (!WindowShouldClose() && !exitWindow) {
//         if (inGame) {
//             game.physics.space.step(1.0f / 60.0f);

//             if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
//                 Vect delta = GetMouseDelta();
//                 camera.target =
//                     Vect(camera.target) + delta.negative() / camera.zoom;
//             }

//             camera.zoom += GetMouseWheelMove() * 0.1f;
//             camera.zoom = ok::partial_clamp(camera.zoom, 0.1f, 3.f);
//         }

//         // draw
//         BeginDrawing();

//         if (inGame) {
//             ClearBackground(WHITE);

//             EndMode3D();

//             // println("{}", playerBody.position());
//             // println("{}", playerShape.asShape().getBoundingBox());
//             BeginMode2D(camera);

//             DrawRectangle(0, 0, 50, 50, BLACK);

//             debugDrawPhysics(game.physics.space, def);

//             EndMode2D();
//         } else {
//             switch (runMainMenu()) {
//             case MenuAction::None:
//                 break;
//             case MenuAction::ExitGame:
//                 exitWindow = true;
//                 break;
//             case MenuAction::EnterGame:
//                 inGame = true;
//                 break;
//             }
//         }

//         EndDrawing();
//     }

//     rlImGuiShutdown();

//     CloseWindow();

//     UnloadShader(def.shader);
//     UnloadTexture(def.maps[MATERIAL_MAP_DIFFUSE].texture);

//     cpSpaceDestroy(&game.physics.space);
//     game.meshes.forEach([](auto item) { UnloadMesh(item.self); });

//     levelArena.destroy();

//     return 0;
// }

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

void initImGui(float mainScale)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(
        mainScale); // Bake a fixed style scale. (until we have a solution for
                    // dynamic style scaling, changing this requires resetting
                    // Style + calling this again)
    // style.FontScaleDpi = mainScale;        // Set initial font scale. (in
    // docking branch: using io.ConfigDpiScaleFonts=true automatically overrides
    // this for every window depending on the current monitor)

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded
    // font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain *
    //   style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to
    // select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr.
    // Please handle those errors in your code (e.g. use an assertion, display
    // an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use
    // FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // style.FontSizeBase = 20.0f;
    // io.Fonts->AddFontDefaultVector();
    // io.Fonts->AddFontDefaultBitmap();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    // IM_ASSERT(font != nullptr);
}

SDL_AppResult SDL_AppInit(void **appstatePointer, int argc, char *argv[])
{
    AppState *appstate = new AppState{GameLib(WERMS_HOTRELOADABLE_DLL_PATH)};
    *appstatePointer = appstate;

    SDL_SetAppMetadata("squinchwerms", "1.0", "com.argus.squinchwerms");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogCritical(int(LoggingCategory::Renderer),
                        "Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Squinchwerms", render_size[0],
                                     render_size[1], SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
        SDL_LogCritical(int(LoggingCategory::Renderer),
                        "Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, render_size[0], render_size[1],
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    initImGui(mainScale);

    // initialize game last so that ImGui context is also initialized
    if (not appstate->gameLib.firstLoad())
        return SDL_APP_FAILURE;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(event);
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; // end + EXIT_SUCCESS
    }

    const bool shouldContinue =
        static_cast<AppState *>(appstate)->gameLib.event(event);

    if (not shouldContinue)
        return SDL_APP_SUCCESS;

    return SDL_APP_CONTINUE;
}

constexpr double ticksToSeconds(uint64_t ticks)
{
    return static_cast<double>(ticks) / 1000.0;
}

SDL_AppResult SDL_AppIterate(void *appstatePointer)
{
    auto *appstate = static_cast<AppState *>(appstatePointer);
    appstate->reloadIfNeeded();

    // start imgui
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    const double now = ticksToSeconds(SDL_GetTicks());
    const float red = (float)(0.5 + 0.5 * SDL_sin(now));
    const float green = (float)(0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    const float blue = (float)(0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
    SDL_SetRenderDrawColorFloat(
        renderer, red, green, blue,
        SDL_ALPHA_OPAQUE_FLOAT); /* new color, full alpha. */
    SDL_RenderClear(renderer);

    const bool shouldContinue = appstate->gameLib.frame(renderer);

    ImGui::Render();
    const ImGuiIO &io = ImGui::GetIO();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);

    if (not shouldContinue)
        return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    delete static_cast<AppState *>(appstate);

    /* SDL will clean up the window/renderer for us. */
}

static std::optional<std::filesystem::file_time_type>
getMostRecentModifyTime(const char *sourceRootPath)
{
    std::optional<std::filesystem::file_time_type> mostRecent;
    for (const auto &dirent :
         std::filesystem::recursive_directory_iterator(sourceRootPath)) {
        if (not dirent.is_regular_file())
            continue;

        const auto path = dirent.path();
        const auto extension = path.extension();

        if (extension != ".cpp" and extension != ".cppm" and extension != ".h")
            continue;

        std::error_code errorCode;
        const auto lastWriteTime =
            std::filesystem::last_write_time(path, errorCode);

        if (errorCode) {
            SDL_LogWarn(int(LoggingCategory::Hotreload),
                        "Unable to read last write time of %s, got error: %s",
                        path.c_str(), errorCode.message().c_str());
            continue;
        }

        if (not mostRecent or lastWriteTime > mostRecent.value())
            mostRecent = lastWriteTime;
    }
    return mostRecent;
}

static bool scanForSourceChanges(const char *sourceRootPath,
                                 const char *gameLibPath,
                                 TimePoint lastRecompilationTime)
{
    const auto sourceLastWriteTime = getMostRecentModifyTime(sourceRootPath);
    if (not sourceLastWriteTime) {
        SDL_LogError(int(LoggingCategory::Hotreload),
                     "Unable to read any source files at directory %s",
                     sourceRootPath);
        return false;
    }

    return std::chrono::file_clock::to_sys(*sourceLastWriteTime) >
           lastRecompilationTime;
}

void AppState::reloadIfNeeded()
{
    if (scanForSourceChanges(WERMS_SOURCE_ROOT_PATH,
                             WERMS_HOTRELOADABLE_DLL_PATH,
                             this->lastHotreloadRecompileTime)) {
        std::error_code errorCode;
        const auto dllLastWriteTime = std::filesystem::last_write_time(
            WERMS_HOTRELOADABLE_DLL_PATH, errorCode);
        std::ignore = std::system(WERMS_HOTRELOAD_BUILD_COMMAND);

        if (!errorCode) {
            const auto recompiledDllLastWriteTime =
                std::filesystem::last_write_time(WERMS_HOTRELOADABLE_DLL_PATH,
                                                 errorCode);
            if (!errorCode) {
                if (dllLastWriteTime != recompiledDllLastWriteTime) {
                    if (this->gameLib.reload()) {
                        SDL_LogInfo(int(LoggingCategory::Hotreload),
                                    "Successfully hotreloaded library %s",
                                    WERMS_HOTRELOADABLE_DLL_PATH);
                    }
                    this->lastHotreloadRecompileTime =
                        std::chrono::system_clock::now();
                } else {
                    SDL_LogError(int(LoggingCategory::Hotreload),
                                 "Did not see a change in the game DLL, "
                                 "aborting hot reload");
                }
            } else {
                SDL_LogError(
                    int(LoggingCategory::Hotreload),
                    "Failed to read write time of file %s, got error: %s",
                    WERMS_HOTRELOADABLE_DLL_PATH, errorCode.message().c_str());
            }
        } else {
            SDL_LogError(int(LoggingCategory::Hotreload),
                         "Failed to read write time of file %s, got error: %s",
                         WERMS_HOTRELOADABLE_DLL_PATH,
                         errorCode.message().c_str());
        }
    }
}
