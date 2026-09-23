#include "projectile.hpp"
#include "global_config.hpp"

#include <cmath>

Projectile::Projectile(const sf::Vector2f& playerPosition, const sf::Vector2f& direction)
    : m_PlayerPosition(playerPosition)
    , m_Direction(direction)
    , m_Speed(config::projectileSpeed)
{
    m_Sprite.setRadius(3.f);
    m_Sprite.setFillColor(sf::Color::Black);
    m_Sprite.setPosition(playerPosition);
}

auto Projectile::GetBounds() const -> sf::FloatRect
{
    return m_Sprite.getGlobalBounds();
}

void Projectile::OnUpdate(float dt)
{
    m_Sprite.move((m_Direction.length() > 0.01f ? m_Direction.normalized() : m_Direction) * m_Speed * dt);
}

auto Projectile::GetPos() const -> sf::Vector2f
{
    return m_Sprite.getPosition();
}

void Projectile::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    const auto pos = m_Sprite.getPosition();
    const auto offset = sf::Vector2f{std::round(pos.x) - pos.x, std::round(pos.y) - pos.y};

    auto snappedStates = states;
    snappedStates.transform.translate(offset);

    target.draw(m_Sprite, snappedStates);
}

void Projectile::SetPosition(const sf::Vector2f& position)
{
    m_Sprite.setPosition(position);
}

void Projectile::SetPlayerPosition(const sf::Vector2f& position)
{
    m_PlayerPosition = position;
}
