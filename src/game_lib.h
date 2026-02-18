#ifndef __WERMS_GAME_LIB_H__
#define __WERMS_GAME_LIB_H__

#include "logging_categories.h"
#include "macros.h"
#include <SDL3/SDL.h>

class GameLib
{
  private:
    using EventCallback = bool (*)(void *ctx, SDL_Event *event);
    using FrameCallback = bool (*)(void *ctx, SDL_Renderer *renderer);
    using InitCallback = void *(*)();

    const char *m_libPath;
    SDL_SharedObject *m_library = nullptr;
    EventCallback m_eventCallback = nullptr;
    FrameCallback m_frameCallback = nullptr;
    InitCallback m_initCallback = nullptr;
    void *m_gameContext = nullptr;

    void unloadIfLoaded()
    {
        if (m_library) {
            SDL_UnloadObject(m_library);
            m_library = nullptr;
        }
    }

  public:
    GameLib() = delete;
    constexpr GameLib(const char *libPath) : m_libPath(libPath) {}

    GameLib(GameLib &&) = delete;
    GameLib(const GameLib &) = delete;
    GameLib &operator=(GameLib &&) = delete;
    GameLib &operator=(const GameLib &) = delete;
    constexpr ~GameLib() { unloadIfLoaded(); }

    [[nodiscard]] bool firstLoad()
    {
        w_assert(not m_gameContext, "firstLoad called multiple times");
        const bool status = reload();

        m_gameContext = m_initCallback();

        return status;
    }

    /// Returns true on success and false on failure
    [[nodiscard]] bool reload()
    {
        unloadIfLoaded();

        m_library = SDL_LoadObject(m_libPath);
        if (!m_library) {
            SDL_LogError(Category_Hotreload,
                         "Failed to hotreload library %s, got error: %s",
                         m_libPath, SDL_GetError());
            return false;
        }

        const auto load = [this]<typename FuncPtr>(FuncPtr &functionPointer,
                                                   const char *symbolName) {
            functionPointer = FuncPtr(SDL_LoadFunction(m_library, symbolName));
            if (!functionPointer) {
                SDL_LogError(Category_Hotreload,
                             "Failed to hotreload symbol %s, got error: %s",
                             symbolName, SDL_GetError());
            }
            return bool(functionPointer);
        };

        if (not load(m_frameCallback, "frame"))
            return false;
        if (not load(m_eventCallback, "event"))
            return false;
        if (not load(m_initCallback, "init"))
            return false;

        return true;
    }

    void frame(SDL_Renderer *renderer)
    {
        if (m_frameCallback) {
            m_frameCallback(m_gameContext, renderer);
        }
    }

	void event(SDL_Event* event)
	{
		if (m_eventCallback) {
			m_eventCallback(m_gameContext, event);
		}
	}
};

#endif
