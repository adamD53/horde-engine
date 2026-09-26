#include "enemy.hpp"
#include "terrain.hpp"

#include <algorithm>
#include <cmath>

namespace horde
{
Enemy::Enemy()
{
    m_Sprite.setSize({50.f, 50.f});
    m_Sprite.setFillColor(sf::Color::Red);

    m_HpBackgroundBar.setSize({44.f, 5.f});
    m_HpBackgroundBar.setFillColor(sf::Color(20, 20, 20));
    m_HpBackgroundBar.setOutlineThickness(1.f);
    m_HpBackgroundBar.setOutlineColor(sf::Color::Black);

    m_HpBar.setSize({44.f, 5.f});
    m_HpBar.setFillColor(sf::Color::Green);
}

void Enemy::Spawn(Entity& target, const sf::Vector2f& spawnPosition, const Terrain& terrain)
{
    m_Target = &target;
    m_Terrain = &terrain;
    m_Health = m_MaxHealth;
    m_AttackTimer = m_AttackCooldown;

    auto dist_speed = std::uniform_int_distribution<int>{50, 100};
    m_Speed = static_cast<float>(dist_speed(s_RandomDevice));

    m_Sprite.setPosition(spawnPosition);

    m_HpBar.setSize({44.f, 5.f});
    m_HpBar.setFillColor(sf::Color::Green);
}

void Enemy::SetTerrain(const Terrain& terrain)
{
    m_Terrain = &terrain;
}

auto Enemy::GetFeetBounds() const -> sf::FloatRect
{
    const auto bounds = GetBounds();
    return {{bounds.position.x + 5.f, bounds.position.y + bounds.size.y - 15.f}, {bounds.size.x - 10.f, 15.f}};
}

void Enemy::SetSwarm(std::span<const std::unique_ptr<Entity>> swarm)
{
    m_Swarm = swarm;
}

auto Enemy::CalculateSeparation() const -> sf::Vector2f
{
    if (m_Swarm.empty())
    {
        return {0.f, 0.f};
    }

    constexpr auto separationRadius = 45.f;
    auto separation = sf::Vector2f{0.f, 0.f};
    const auto currentPos = GetPos();

    for (const auto& peer : m_Swarm)
    {
        if (peer.get() == this)
        {
            continue;
        }

        const auto diff = currentPos - peer->GetPos();
        const auto distance = diff.length();

        if (distance > 0.001f && distance < separationRadius)
        {
            separation += (diff / distance) * (1.f - (distance / separationRadius));
        }
    }

    return separation;
}

auto Enemy::GetBounds() const -> sf::FloatRect
{
    return m_Sprite.getGlobalBounds();
}

auto Enemy::GetPos() const -> sf::Vector2f
{
    return m_Sprite.getPosition();
}

void Enemy::TakeDamage(float amount)
{
    m_Health = std::max(0.f, m_Health - amount);
}

auto Enemy::IsDead() const -> bool
{
    return m_Health <= 0.f;
}

auto Enemy::GetHealth() const -> float
{
    return m_Health;
}

auto Enemy::GetMaxHealth() const -> float
{
    return m_MaxHealth;
}

void Enemy::OnUpdate(float dt)
{
    if (!m_Target) return;

    const auto toTarget = m_Target->GetPos() - (GetPos() + sf::Vector2f(10.f, 10.f));
    const auto chaseDirection = toTarget.length() > 0.001f ? toTarget.normalized() : sf::Vector2f{0.f, 0.f};

    const auto separationForce = CalculateSeparation();
    constexpr auto separationWeight = 1.5f;

    m_Direction = chaseDirection + (separationForce * separationWeight);

    if (m_Direction.length() > 0.001f)
    {
        auto movement = m_Direction.normalized() * (dt * m_Speed);
        if (m_Terrain)
        {
            movement = m_Terrain->ResolveMovement(GetFeetBounds(), movement);
        }
        m_Sprite.move(movement);
    }

    const auto hpPos = GetPos() + sf::Vector2f(3.f, -10.f);
    m_HpBackgroundBar.setPosition(hpPos);
    m_HpBar.setPosition(hpPos);

    const auto healthRatio = std::clamp(m_Health / m_MaxHealth, 0.f, 1.f);
    m_HpBar.setSize({44.f * healthRatio, 5.f});

    m_AttackTimer += dt;
    if (m_AttackTimer >= m_AttackCooldown && GetBounds().findIntersection(m_Target->GetBounds()))
    {
        constexpr auto attackDamage = 10.0f;
        m_Target->TakeDamage(attackDamage);
        m_AttackTimer = 0.0f;
    }
}

void Enemy::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    const auto pos = m_Sprite.getPosition();
    const auto offset = sf::Vector2f{std::round(pos.x) - pos.x, std::round(pos.y) - pos.y};

    auto snappedStates = states;
    snappedStates.transform.translate(offset);

    target.draw(m_Sprite, snappedStates);
    target.draw(m_HpBackgroundBar, snappedStates);
    target.draw(m_HpBar, snappedStates);
}
}

