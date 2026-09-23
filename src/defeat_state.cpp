#include "defeat_state.hpp"
#include "menu_state.hpp"
#include "state_machine.hpp"

#include <memory>

namespace horde
{
DefeatState::DefeatState(StateMachine& stateMachine, sf::RenderWindow& window)
    : m_StateMachine(stateMachine)
    , m_Window(window)
{
    const auto windowSize = sf::Vector2f{m_Window.getSize()};
    const auto buttonSize = sf::Vector2f{260.0f, 60.0f};

    m_MenuButton.setSize(buttonSize);
    m_MenuButton.setOrigin(buttonSize / 2.0f);
    m_MenuButton.setPosition(windowSize / 2.0f);
    m_MenuButton.setFillColor(sf::Color{140, 30, 30});
    m_MenuButton.setOutlineThickness(3.0f);
    m_MenuButton.setOutlineColor(sf::Color::White);
}

void DefeatState::OnEnter()
{
}

void DefeatState::OnExit()
{
}

void DefeatState::OnUpdate([[maybe_unused]] float dt)
{
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
    window.draw(m_MenuButton);
}

void DefeatState::OnEvent(const sf::Event& event)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Space || keyPressed->code == sf::Keyboard::Key::Enter)
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
    m_StateMachine.ChangeState(std::make_unique<MenuState>(m_StateMachine, m_Window));
}
}
