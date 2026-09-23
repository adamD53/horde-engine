#include "game.hpp"

#include <SFML/Graphics.hpp>

auto main() -> int
{
    const auto specification = GameSpecification{1280, 720, "title"};

    auto game = Game{specification};
    game.Run();
}
