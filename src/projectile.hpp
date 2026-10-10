#pragma once

#include "entity.hpp"

namespace horde
{
class Projectile : public Entity
{
public:
    Projectile();
    ~Projectile() = default;

    void Spawn(const sf::Vector2f& playerPosition, const sf::Vector2f& direction);

    void SetPosition(const sf::Vector2f& position);
    void SetPlayerPosition(const sf::Vector2f& position);
    void OnUpdate(float dt) override;
    [[nodiscard]] auto GetPos() const -> sf::Vector2f override;
    [[nodiscard]] auto GetBounds() const -> sf::FloatRect override;
    [[nodiscard]] auto IsDead() const -> bool override;
    void SetDead(bool dead);

private:
    sf::Vector2f m_PlayerPosition;
    sf::Vector2f m_Direction;
    float m_Speed;
    bool m_IsDead = false;
    sf::CircleShape m_Sprite;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
}
