#pragma once

#include "entity.hpp"

class Projectile : public Entity
{
public:
    Projectile(const sf::Vector2f& playerPosition, const sf::Vector2f& direction);
    ~Projectile() = default;

    void SetPosition(const sf::Vector2f& position);
    void SetPlayerPosition(const sf::Vector2f& position);
    void OnUpdate(float dt) override;
    auto GetPos() const -> sf::Vector2f override;
    auto GetBounds() const -> sf::FloatRect override;

private:
    sf::Vector2f m_PlayerPosition;
    sf::Vector2f m_Direction;
    float m_Speed;
    sf::CircleShape m_Sprite;
    sf::Texture m_Texture;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
