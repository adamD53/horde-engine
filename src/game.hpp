#pragma once

#include "debug_ui.hpp"
#include "game_settings.hpp"
#include "state_machine.hpp"

#include <SFML/Graphics.hpp>

#include <cstdint>
#include <string>

namespace horde
{
struct GameSpecification
{
    const uint32_t width, height;
    const std::string title;
};

class Game
{
public:
    Game(const GameSpecification& spec);
    ~Game();
    void Run();

    void ApplySettings(const GameSettings& newSettings, bool forceRecreate = false);
    [[nodiscard]] auto GetSettings() -> GameSettings&;
    [[nodiscard]] auto GetSettings() const -> const GameSettings&;

private:
    std::string m_Title;
    GameSettings m_Settings{};
    sf::RenderWindow m_Window;
    StateMachine m_StateMachine;
    DebugUI m_DebugUI;
    bool m_ImGuiInitialized{false};
};
}
