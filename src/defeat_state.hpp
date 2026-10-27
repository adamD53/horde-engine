#pragma once

#include "game_settings.hpp"
#include "state.hpp"

#include <SFML/Graphics.hpp>

#include <functional>
#include <optional>

namespace horde
{
class StateMachine;

class DefeatState : public State
{
public:
    using ApplySettingsCallback = std::function<void(const GameSettings&)>;

    DefeatState(
        StateMachine& stateMachine,
        sf::RenderWindow& window,
        GameSettings& settings,
        ApplySettingsCallback onApplySettings = {});
    DefeatState(StateMachine& stateMachine, sf::RenderWindow& window);
    ~DefeatState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(sf::RenderWindow& window) override;
    void OnEvent(const sf::Event& event) override;

private:
    void ReturnToMenu();
    void RebuildLayout();

private:
    StateMachine& m_StateMachine;
    sf::RenderWindow& m_Window;
    GameSettings m_FallbackSettings{};
    GameSettings& m_Settings;
    ApplySettingsCallback m_OnApplySettings{};

    sf::Font m_Font{};
    bool m_HasFont{false};
    std::optional<sf::Text> m_TitleText{};
    sf::RectangleShape m_MenuButton{};
    std::optional<sf::Text> m_ButtonText{};
};
}
