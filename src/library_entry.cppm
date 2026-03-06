module;

#include "game_lib.h"

export module entry;
import main_menu;
import logging;
import box2d;

struct Context
{
	b2::WorldID world;
	b2::BodyID floor;
};

extern "C"
{
    // called once at startup, we return the ctx which will be passed on all
    // future calls
    void *init()
    {
        lg::info(lg::Category::Gameplay, "gamelib init() called");
        return nullptr;
    }

    void onHotReload(const hotreload::GlobalContext *context)
    {
        ImGui::SetCurrentContext(context->imguiContext);
        ImGui::SetAllocatorFunctions(context->imguiAlloc, context->imguiFree,
                                     context->imguiAllocUsrData);
    }

    /// Called every frame
    /// return true if continue, false if quit
    bool frame(void *ctx, SDL_Renderer *renderer)
    {
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
    bool event(void *ctx, SDL_Event *event) { return true; }
}
