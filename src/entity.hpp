#pragma once

#include <SFML/Graphics.hpp>

class Entity : public sf::Drawable
{
public:
    virtual ~Entity() = default;

    virtual void OnEvent() {}
    virtual void OnUpdate(float dt) {}
    virtual auto GetPos() const -> sf::Vector2f = 0;
    virtual auto GetBounds() const -> sf::FloatRect = 0;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override = 0;
};
