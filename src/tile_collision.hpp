#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <vector>

auto BuildOpaqueTileCollision(const sf::Image& image, sf::IntRect tile) -> std::vector<sf::FloatRect>;
