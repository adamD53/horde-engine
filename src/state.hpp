#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

namespace horde
{
class DebugUI;

class State
{
public:
    virtual ~State() = default;

    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void OnUpdate(float dt) = 0;
    virtual void OnRender(sf::RenderWindow& window) = 0;
    virtual void OnImGuiRender([[maybe_unused]] const DebugUI& debugUI) {}
    virtual void OnEvent(const sf::Event& event) = 0;
};
}
