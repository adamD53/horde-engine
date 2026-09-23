#pragma once

#include "state.hpp"

#include <SFML/Graphics.hpp>

namespace horde
{
class StateMachine;

class MenuState : public State
{
public:
    MenuState(StateMachine& stateMachine, sf::RenderWindow& window);
    ~MenuState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(sf::RenderWindow& window) override;
    void OnEvent(const sf::Event& event) override;

private:
    void StartGame();

private:
    StateMachine& m_StateMachine;
    sf::RenderWindow& m_Window;
    sf::RectangleShape m_PlayButton;
};
}
