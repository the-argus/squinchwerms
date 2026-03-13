module;

#include "game_lib.h"

export module entry;
import main_menu;
import logging;
import opt;
import box2d;
import aliases;
import reflection;

struct Context
{
    b2::WorldID world;
    b2::BodyID floor;
    Opt<b2::Vec2U32> windowSize{}; // changes on events
};

extern "C"
{
    // called once at startup, we return the ctx which will be passed on all
    // future calls
    void *init()
    {
        tests::reflection();

        lg::info(lg::Category::Gameplay, "gamelib init() called");
        return new Context;
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
