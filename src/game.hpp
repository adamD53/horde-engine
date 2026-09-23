#pragma once

#include "state_machine.hpp"

#include <SFML/Graphics.hpp>

#include <cstdint>
#include <string>

struct GameSpecification
{
    const uint32_t width, height;
    const std::string title;
};

class Game
{
public:
    Game(const GameSpecification& spec);
    ~Game() = default;
    void Run();

private:
    sf::RenderWindow m_Window;
    StateMachine m_StateMachine;
};
