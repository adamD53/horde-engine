#include "game.hpp"

#include <SFML/Graphics.hpp>

auto main() -> int
{
    const auto specification = horde::GameSpecification{2560, 1440, "title"};

    auto game = horde::Game{specification};
    game.Run();
}
