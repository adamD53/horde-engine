#include "menu_state.hpp"
#include "game_state.hpp"
#include "state_machine.hpp"

#include <memory>

MenuState::MenuState(StateMachine& stateMachine, sf::RenderWindow& window)
    : m_StateMachine(stateMachine), m_Window(window)
{
    const auto windowSize = sf::Vector2f{m_Window.getSize()};
    const auto buttonSize = sf::Vector2f{240.0f, 60.0f};

    m_PlayButton.setSize(buttonSize);
    m_PlayButton.setOrigin(buttonSize / 2.0f);
    m_PlayButton.setPosition(windowSize / 2.0f);
    m_PlayButton.setFillColor(sf::Color{70, 130, 180});
    m_PlayButton.setOutlineThickness(3.0f);
    m_PlayButton.setOutlineColor(sf::Color::White);
}

void MenuState::OnEnter()
{
}

void MenuState::OnExit()
{
}

void MenuState::OnUpdate(float dt)
{
    (void)dt;

    const auto mousePos = m_Window.mapPixelToCoords(sf::Mouse::getPosition(m_Window));
    if (m_PlayButton.getGlobalBounds().contains(mousePos))
    {
        m_PlayButton.setFillColor(sf::Color{100, 160, 210});
    }
    else
    {
        m_PlayButton.setFillColor(sf::Color{70, 130, 180});
    }
}

void MenuState::OnRender(sf::RenderWindow& window)
{
    window.setView(window.getDefaultView());
    window.clear(sf::Color{30, 30, 35});
    window.draw(m_PlayButton);
}

void MenuState::OnEvent(const sf::Event& event)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Space || keyPressed->code == sf::Keyboard::Key::Enter)
        {
            StartGame();
        }
    }
    else if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mousePressed->button == sf::Mouse::Button::Left)
        {
            const auto clickPos = m_Window.mapPixelToCoords(mousePressed->position);
            if (m_PlayButton.getGlobalBounds().contains(clickPos))
            {
                StartGame();
            }
        }
    }
}

void MenuState::StartGame()
{
    m_StateMachine.ChangeState(std::make_unique<GameState>(m_StateMachine, m_Window));
}
