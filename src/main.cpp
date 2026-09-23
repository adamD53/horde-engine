#include "game.hpp"

#include <SFML/Graphics.hpp>

auto main() -> int
{
    const auto specification = horde::GameSpecification{1280, 720, "title"};

    auto game = horde::Game{specification};
    game.Run();
}
