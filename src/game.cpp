#include "game.hpp"
#include "menu_state.hpp"

#include <memory>

Game::Game(const GameSpecification& spec)
{
    m_Window.create(sf::VideoMode(sf::Vector2u(spec.width, spec.height)), spec.title);
    m_Window.setVerticalSyncEnabled(true);

    m_StateMachine.ChangeState(std::make_unique<MenuState>(m_StateMachine, m_Window));
    m_StateMachine.ProcessStateChange();
}

void Game::Run()
{
    auto clock = sf::Clock{};

    while (m_Window.isOpen())
    {
        m_StateMachine.ProcessStateChange();

        while (const std::optional event = m_Window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                m_Window.close();
            }

            if (auto* state = m_StateMachine.GetActiveState())
            {
                state->OnEvent(*event);
            }
        }

        m_StateMachine.ProcessStateChange();

        const auto dt = clock.restart().asSeconds();

        if (auto* state = m_StateMachine.GetActiveState())
        {
            state->OnUpdate(dt);
        }

        m_Window.clear();

        if (auto* state = m_StateMachine.GetActiveState())
        {
            state->OnRender(m_Window);
        }

        m_Window.display();
    }
}
