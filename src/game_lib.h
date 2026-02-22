#ifndef __WERMS_GAME_LIB_H__
#define __WERMS_GAME_LIB_H__

#include "logging_categories.h"
#include "macros.h"
#include <SDL3/SDL.h>
#include <imgui.h>

namespace hotreload {

/// Stuff that needs to be restored / consistent across the DLL boundary
struct GlobalContext
{
    ImGuiContext *imguiContext;
    ImGuiMemAllocFunc imguiAlloc;
    ImGuiMemFreeFunc imguiFree;
    void *imguiAllocUsrData;
};

using EventCallback = bool (*)(void *ctx, SDL_Event *event);
using FrameCallback = bool (*)(void *ctx, SDL_Renderer *renderer);
using HotReloadedCallback = bool (*)(const GlobalContext *);
using InitCallback = void *(*)();
} // namespace hotreload

class GameLib
{
  private:
    const char *m_libPath;
    SDL_SharedObject *m_library = nullptr;
    hotreload::EventCallback m_eventCallback = nullptr;
    hotreload::FrameCallback m_frameCallback = nullptr;
    hotreload::InitCallback m_initCallback = nullptr;
    hotreload::HotReloadedCallback m_onHotReloadCallback = nullptr;
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
    ~GameLib() { unloadIfLoaded(); }

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
            SDL_LogError(int(LoggingCategory::Hotreload),
                         "Failed to hotreload library %s, got error: %s",
                         m_libPath, SDL_GetError());
            return false;
        }

        const auto load = [this]<typename FuncPtr>(FuncPtr &functionPointer,
                                                   const char *symbolName) {
            functionPointer = FuncPtr(SDL_LoadFunction(m_library, symbolName));
            if (!functionPointer) {
                SDL_LogError(int(LoggingCategory::Hotreload),
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
        if (not load(m_onHotReloadCallback, "onHotReload"))
            return false;
        else {
            hotreload::GlobalContext ctx{
                .imguiContext = ImGui::GetCurrentContext(),
            };
            ImGui::GetAllocatorFunctions(&ctx.imguiAlloc, &ctx.imguiFree,
                                         &ctx.imguiAllocUsrData);
            m_onHotReloadCallback(&ctx);
        }

        return true;
    }

    [[nodiscard]] bool frame(SDL_Renderer *renderer)
    {
        if (m_frameCallback) {
            return m_frameCallback(m_gameContext, renderer);
        }
        return true;
    }

    [[nodiscard]] bool event(SDL_Event *event)
    {
        if (m_eventCallback) {
            return m_eventCallback(m_gameContext, event);
        }
        return true;
    }
};

#endif
