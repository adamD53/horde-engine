#include "defeat_state.hpp"
#include "menu_state.hpp"
#include "state_machine.hpp"

#include <imgui.h>

#include <iostream>
#include <memory>
#include <utility>

namespace horde
{
DefeatState::DefeatState(
    StateMachine& stateMachine,
    sf::RenderWindow& window,
    GameSettings& settings,
    ApplySettingsCallback onApplySettings)
    : m_StateMachine(stateMachine)
    , m_Window(window)
    , m_Settings(settings)
    , m_OnApplySettings(std::move(onApplySettings))
{
    m_HasFont = m_Font.openFromFile(RESOURCES_PATH "fonts/font.ttf");
    if (!m_HasFont)
    {
        std::cerr << "Warning: DefeatState could not load font from " << RESOURCES_PATH "fonts/font.ttf\n";
    }

    RebuildLayout();
}

DefeatState::DefeatState(StateMachine& stateMachine, sf::RenderWindow& window)
    : m_StateMachine(stateMachine)
    , m_Window(window)
    , m_FallbackSettings(SettingsSerializer::LoadFromFile(GameSettings::GetDefaultPath()))
    , m_Settings(m_FallbackSettings)
{
    m_HasFont = m_Font.openFromFile(RESOURCES_PATH "fonts/font.ttf");
    if (!m_HasFont)
    {
        std::cerr << "Warning: DefeatState could not load font from " << RESOURCES_PATH "fonts/font.ttf\n";
    }

    RebuildLayout();
}

void DefeatState::RebuildLayout()
{
    const auto windowSize = sf::Vector2f{m_Window.getSize()};
    const auto center = windowSize / 2.0f;
    const auto buttonSize = sf::Vector2f{260.0f, 60.0f};

    m_MenuButton.setSize(buttonSize);
    m_MenuButton.setOrigin(buttonSize / 2.0f);
    m_MenuButton.setPosition({center.x, center.y + 30.0f});
    m_MenuButton.setFillColor(sf::Color{140, 30, 30});
    m_MenuButton.setOutlineThickness(3.0f);
    m_MenuButton.setOutlineColor(sf::Color::White);

    if (m_HasFont)
    {
        m_TitleText.emplace(m_Font, "DEFEAT", 64);
        m_TitleText->setFillColor(sf::Color{235, 65, 65});
        m_TitleText->setOutlineThickness(3.0f);
        m_TitleText->setOutlineColor(sf::Color{20, 5, 5});
        const auto titleBounds = m_TitleText->getLocalBounds();
        m_TitleText->setOrigin({titleBounds.position.x + titleBounds.size.x / 2.0f,
                                titleBounds.position.y + titleBounds.size.y / 2.0f});
        m_TitleText->setPosition({center.x, center.y - 80.0f});

        m_ButtonText.emplace(m_Font, "Main Menu", 26);
        m_ButtonText->setFillColor(sf::Color::White);
        const auto btnBounds = m_ButtonText->getLocalBounds();
        m_ButtonText->setOrigin({btnBounds.position.x + btnBounds.size.x / 2.0f,
                                 btnBounds.position.y + btnBounds.size.y / 2.0f});
        m_ButtonText->setPosition(m_MenuButton.getPosition());
    }
}

void DefeatState::OnEnter()
{
}

void DefeatState::OnExit()
{
}

void DefeatState::OnUpdate([[maybe_unused]] float dt)
{
    const bool imguiFocused =
        ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || ImGui::IsAnyItemActive();
    if (imguiFocused)
    {
        m_MenuButton.setFillColor(sf::Color{140, 30, 30});
        return;
    }

    const auto mousePos = m_Window.mapPixelToCoords(sf::Mouse::getPosition(m_Window));
    if (m_MenuButton.getGlobalBounds().contains(mousePos))
    {
        m_MenuButton.setFillColor(sf::Color{180, 50, 50});
    }
    else
    {
        m_MenuButton.setFillColor(sf::Color{140, 30, 30});
    }
}

void DefeatState::OnRender(sf::RenderWindow& window)
{
    window.setView(window.getDefaultView());
    window.clear(sf::Color{35, 10, 10});
    if (m_TitleText)
    {
        window.draw(*m_TitleText);
    }
    window.draw(m_MenuButton);
    if (m_ButtonText)
    {
        window.draw(*m_ButtonText);
    }
}

void DefeatState::OnEvent(const sf::Event& event)
{
    if (const auto* resized = event.getIf<sf::Event::Resized>())
    {
        m_Window.setView(sf::View{sf::FloatRect{{0.0f, 0.0f}, sf::Vector2f{resized->size}}});
        RebuildLayout();
        return;
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Space ||
            keyPressed->code == sf::Keyboard::Key::Enter ||
            keyPressed->code == sf::Keyboard::Key::Escape)
        {
            ReturnToMenu();
        }
    }
    else if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mousePressed->button == sf::Mouse::Button::Left)
        {
            const auto clickPos = m_Window.mapPixelToCoords(mousePressed->position);
            if (m_MenuButton.getGlobalBounds().contains(clickPos))
            {
                ReturnToMenu();
            }
        }
    }
}

void DefeatState::ReturnToMenu()
{
    m_StateMachine.ChangeState(
        std::make_unique<MenuState>(m_StateMachine, m_Window, m_Settings, m_OnApplySettings));
}
} // namespace horde
