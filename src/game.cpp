#include "game.hpp"
#include "menu_state.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>

namespace horde
{
Game::Game(const GameSpecification& spec)
{
    m_Window.create(sf::VideoMode(sf::Vector2u(spec.width, spec.height)), spec.title);
    m_Window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(m_Window))
    {
        std::cerr << "Failed to initialize ImGui-SFML backend.\n";
    }

    m_StateMachine.ChangeState(std::make_unique<MenuState>(m_StateMachine, m_Window));
    m_StateMachine.ProcessStateChange();
}

Game::~Game()
{
    ImGui::SFML::Shutdown(m_Window);
}

void Game::Run()
{
    auto clock = sf::Clock{};

    while (m_Window.isOpen())
    {
        m_StateMachine.ProcessStateChange();

        while (const std::optional event = m_Window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(m_Window, *event);

            if (event->is<sf::Event::Closed>())
            {
                m_Window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::F1)
                {
                    m_DebugUI.Toggle();
                }
            }

            if (auto* state = m_StateMachine.GetActiveState())
            {
                state->OnEvent(*event);
            }
        }

        m_StateMachine.ProcessStateChange();

        const auto dt = std::min(clock.restart().asSeconds(), 0.1f);

        ImGui::SFML::Update(m_Window, sf::seconds(dt));

        if (auto* state = m_StateMachine.GetActiveState())
        {
            state->OnUpdate(dt);
        }

        if (auto* state = m_StateMachine.GetActiveState())
        {
            state->OnRender(m_Window);
            state->OnImGuiRender();
        }

        m_DebugUI.Render();

        ImGui::SFML::Render(m_Window);

        m_Window.display();
    }
}
}
