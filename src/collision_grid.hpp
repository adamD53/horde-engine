#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <vector>

class CollisionGrid
{
public:
    void Reset(unsigned width, unsigned height, sf::Vector2f tileSize);
    void Add(unsigned x, unsigned y, sf::FloatRect bounds);
    auto Move(sf::FloatRect body, sf::Vector2f delta) const -> sf::Vector2f;

private:
    auto Sweep(const sf::FloatRect& body, float distance, bool horizontal) const -> float;

    unsigned m_Width = 0, m_Height = 0;
    sf::Vector2f m_TileSize;
    std::vector<std::vector<sf::FloatRect>> m_Cells;
};
