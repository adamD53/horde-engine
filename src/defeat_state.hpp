#pragma once

#include "state.hpp"

#include <SFML/Graphics.hpp>

namespace horde
{
class StateMachine;

class DefeatState : public State
{
public:
    DefeatState(StateMachine& stateMachine, sf::RenderWindow& window);
    ~DefeatState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(sf::RenderWindow& window) override;
    void OnEvent(const sf::Event& event) override;

private:
    void ReturnToMenu();

private:
    StateMachine& m_StateMachine;
    sf::RenderWindow& m_Window;
    sf::RectangleShape m_MenuButton;
};
}
