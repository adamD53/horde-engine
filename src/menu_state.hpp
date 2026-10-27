#pragma once

#include "game_settings.hpp"
#include "state.hpp"

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace horde
{
class StateMachine;

class MenuState : public State
{
public:
    using ApplySettingsCallback = std::function<void(const GameSettings&)>;

    MenuState(
        StateMachine& stateMachine,
        sf::RenderWindow& window,
        GameSettings& settings,
        ApplySettingsCallback onApplySettings = {});
    MenuState(StateMachine& stateMachine, sf::RenderWindow& window);
    ~MenuState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(sf::RenderWindow& window) override;
    void OnEvent(const sf::Event& event) override;

private:
    enum class SubMenu : uint8_t
    {
        Main = 0,
        Settings = 1
    };

    struct Button
    {
        sf::RectangleShape shape{};
        std::optional<sf::Text> label{};
        sf::Color baseColor{70, 130, 180};
        sf::Color hoverColor{100, 160, 210};
        std::function<void()> onClick{};
    };

    struct CycleRow
    {
        std::optional<sf::Text> label{};
        std::optional<sf::Text> valueText{};
        Button prevButton{};
        Button nextButton{};
        bool enabled{true};
    };

    struct SliderRow
    {
        std::optional<sf::Text> label{};
        std::optional<sf::Text> valueText{};
        sf::RectangleShape track{};
        sf::RectangleShape fill{};
        sf::CircleShape knob{};
        float value{100.0f};
        bool dragging{false};
        std::function<void(float)> onValueChanged{};
    };

    void StartGame();
    void OpenSettings();
    void CloseSettings();
    void ApplyPendingSettings();

    void RebuildLayout();
    void RefreshSettingsValues();
    void UpdateSliderFromMouse(SliderRow& slider, float mouseX);

private:
    StateMachine& m_StateMachine;
    sf::RenderWindow& m_Window;
    GameSettings m_FallbackSettings{};
    GameSettings& m_Settings;
    GameSettings m_PendingSettings{};
    ApplySettingsCallback m_OnApplySettings{};

    SubMenu m_SubMenu{SubMenu::Main};
    sf::Font m_Font{};
    bool m_HasFont{false};

    std::optional<sf::Text> m_TitleText;
    std::vector<Button> m_MainButtons;

    sf::RectangleShape m_SettingsPanel;
    std::optional<sf::Text> m_SettingsHeader;

    std::vector<Resolution> m_AvailableResolutions;
    size_t m_ResolutionIndex{0};
    size_t m_FrameLimitIndex{0};

    CycleRow m_WindowModeRow;
    CycleRow m_ResolutionRow;
    CycleRow m_VsyncRow;
    CycleRow m_FrameLimitRow;
    CycleRow m_StatsOverlayRow;

    SliderRow m_MasterVolumeRow;
    SliderRow m_MusicVolumeRow;
    SliderRow m_SfxVolumeRow;

    Button m_ApplyButton;
    Button m_BackButton;
};
} // namespace horde
