module;

#include <imgui.h>

export module main_menu;

export enum class MenuAction
{
    EnterGame,
    None,
    ExitGame,
};

export MenuAction runMainMenu() noexcept
{
    ImGui::Begin("Main Menu");

    auto action = MenuAction::None;
    auto noOtherButtonsPressed = [&] { return action == MenuAction::None; };

    using namespace ImGui;

    ImGui::BeginGroup();
    {
        if (ImGui::Button("Squinch")) {
            action = MenuAction::EnterGame;
        }

        if (ImGui::Button("Exit Game") && noOtherButtonsPressed()) {
            action = MenuAction::ExitGame;
        }
    }
    ImGui::EndGroup();
    ImGui::End();

    return action;
}
