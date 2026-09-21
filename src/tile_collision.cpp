#include "tile_collision.hpp"

#include <algorithm>
#include <stdexcept>

auto BuildOpaqueTileCollision(const sf::Image& image, sf::IntRect tile) -> std::vector<sf::FloatRect>
{
    const auto size = image.getSize();
    if (tile.position.x < 0 || tile.position.y < 0 || tile.size.x <= 0 || tile.size.y <= 0 ||
        tile.position.x + tile.size.x > static_cast<int>(size.x) ||
        tile.position.y + tile.size.y > static_cast<int>(size.y))
        throw std::runtime_error("Collision tile lies outside its image");

    auto shapes = std::vector<sf::FloatRect>{};
    for (auto y = 0; y < tile.size.y; ++y)
    {
        auto x = 0;
        const auto opaque = [&](auto column)
        {
            return image.getPixel({static_cast<unsigned>(tile.position.x + column),
                                   static_cast<unsigned>(tile.position.y + y)}).a != 0;
        };
        while (x < tile.size.x)
        {
            if (!opaque(x)) { ++x; continue; }
            const auto start = x;
            while (x < tile.size.x && opaque(x)) ++x;
            const auto width = static_cast<float>(x - start);
            const auto previous = std::find_if(shapes.begin(), shapes.end(), [&](const auto& shape)
            {
                return shape.position.x == start && shape.size.x == width &&
                       shape.position.y + shape.size.y == y;
            });
            if (previous != shapes.end()) previous->size.y += 1.f;
            else shapes.emplace_back(sf::Vector2f(static_cast<float>(start), static_cast<float>(y)),
                                     sf::Vector2f(width, 1.f));
        }
    }
    return shapes;
}
