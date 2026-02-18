module;

#include "logging_categories.h"
#include <SDL3/SDL.h>

export module entry;

extern "C"
{
    // called once at startup, we return the ctx which will be passed on all
    // future calls
    void *init() { return nullptr; }

    /// Called every frame
    /// return true if continue, false if quit
    bool frame(void *ctx, SDL_Renderer *renderer)
    {
        // SDL_LogInfo(Category_Renderer, "entered game lib frame() function");
        return true;
    }

    /// Called on OS event
    /// return true if continue, false if quit
    bool event(void *ctx, SDL_Event *event) { return true; }
}
