module;

#include "game_lib.h"

export module entry;
import main_menu;
import logging;
import opt;
import physics;
import aliases;
import reflection;
import json;
import box2d;

struct Context
{
    World world;
    Body floor;
    Body square;
    Opt<b2::Vec2U32> windowSize{}; // changes on events
};

extern "C"
{
    // called once at startup, we return the ctx which will be passed on all
    // future calls
    void *init()
    {
        tests::reflection();
        tests::json();

        lg::info(lg::Category::Gameplay, "gamelib init() called");
        const auto world = World::createWorld({});
        auto *out = new Context{
            .world = world,
            .floor = world.createBody({
                .type = BodyType::Static,
                .position = Vec2{.x = 0, .y = -10},
                .name = "floor",
            }),
            .square = world.createBody({
                .type = BodyType::Kinematic,
                .position = Vec2{.x = 0, .y = 10},
                .name = "square",
            }),
        };

        constexpr Polygon squarePolygon = {
            .vertices = {{-1, 1}, {-1, -1}, {1, -1}, {1, 1}},
            .normals = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}},
            .centroid = {},
            .count = 4,
        };
        out->square.addPolygonShape({}, squarePolygon);
        out->floor.addSegmentShape({}, {.point1 = {-10, 0}, .point2 = {10, 0}});

        return out;
    }

    void onHotReload(const hotreload::GlobalContext *context)
    {
        ImGui::SetCurrentContext(context->imguiContext);
        ImGui::SetAllocatorFunctions(context->imguiAlloc, context->imguiFree,
                                     context->imguiAllocUsrData);
    }

    /// Called every frame
    /// return true if continue, false if quit
    bool frame(void *context, SDL_Renderer *renderer)
    {
        auto *const ctx = static_cast<Context *>(context);

        // TODO: put correct frame timestep here? honestly I'm not sure how
        // physics process vs. frame process is usually implemented. I guess it
        // could be an event on a timer? or just force vsync and use frametime
        // here... but then maybe things become unstable at low framerates
        ctx->world.step(1.0f / 60.0f);

        if (ctx->windowSize) {
            SDL_SetRenderLogicalPresentation(
                renderer, ctx->windowSize->x, ctx->windowSize->y,
                SDL_LOGICAL_PRESENTATION_LETTERBOX);
            ctx->windowSize = null;
        }
        switch (runMainMenu()) {
        case MenuAction::EnterGame:
            break;
        case MenuAction::ExitGame:
            return false;
        default:
            break;
        }
        // SDL_LogInfo(Renderer, "entered game lib frame() function");
        return true;
    }

    /// Called on OS event
    /// return true if continue, false if quit
    bool event(void *context, SDL_Event *event)
    {
        auto *const ctx = static_cast<Context *>(context);
        switch (event->type) {
        case SDL_EVENT_WINDOW_RESIZED: {
            const i32 newWidth = event->window.data1;
            const i32 newHeight = event->window.data2;
            ctx->windowSize = b2::Vec2U32{
                .x = static_cast<u32>(newWidth),
                .y = static_cast<u32>(newHeight),
            };
        }
        default:
            return true;
        }
    }
}
