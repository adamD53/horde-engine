#include "enemy.hpp"

#include <cmath>

Enemy::Enemy(Entity& target)
    : m_Target(&target)
{
    auto dist = std::uniform_real_distribution<float>{0.0f, 1000.0f};
    auto dist_speed = std::uniform_int_distribution<int>{50, 100};
    const auto randomX = dist(s_RandomDevice);
    const auto randomY = dist(s_RandomDevice);
    m_Speed = static_cast<float>(dist_speed(s_RandomDevice));

    m_Sprite.setSize({50, 50});
    m_Sprite.setPosition({randomX, randomY});
    m_Sprite.setFillColor(sf::Color::Red);
}

auto Enemy::GetBounds() const -> sf::FloatRect
{
    return m_Sprite.getGlobalBounds();
}

auto Enemy::GetPos() const -> sf::Vector2f
{
    return m_Sprite.getPosition();
}

void Enemy::OnUpdate(float dt)
{
    if (!m_Target)
        return;
    m_Direction = m_Target->GetPos() - (GetPos() + sf::Vector2f(10.f, 10.f));
    if (m_Direction.length() > 0.001f)
    {
        m_Sprite.move(m_Direction.normalized() * (dt * m_Speed));
    }
}

void Enemy::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    const auto pos = m_Sprite.getPosition();
    const auto offset = sf::Vector2f{std::round(pos.x) - pos.x, std::round(pos.y) - pos.y};

    auto snappedStates = states;
    snappedStates.transform.translate(offset);

    target.draw(m_Sprite, snappedStates);
}
