#include "enemy.hpp"
#include "global_config.hpp"
#include "terrain.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace horde
{
auto Enemy::GetTexture() -> const sf::Texture&
{
    static sf::Texture texture;
    static bool loaded = false;
    if (!loaded)
    {
        if (!texture.loadFromFile(RESOURCES_PATH "enemies/bat.png"))
        {
            throw std::runtime_error("Failed to load bat texture: " RESOURCES_PATH "enemies/bat.png");
        }
        loaded = true;
    }
    return texture;
}

Enemy::Enemy()
    : m_Sprite(GetTexture())
{
    m_Sprite.setOrigin(s_SpriteOrigin);

    m_Animator.AddState(EnemyState::FlyUp, {0, 0, s_FrameCount, s_FrameDuration, true});
    m_Animator.AddState(EnemyState::FlyRight, {1, 0, s_FrameCount, s_FrameDuration, true});
    m_Animator.AddState(EnemyState::FlyDown, {2, 0, s_FrameCount, s_FrameDuration, true});
    m_Animator.AddState(EnemyState::FlyLeft, {3, 0, s_FrameCount, s_FrameDuration, true});

    m_Animator.Play(EnemyState::FlyDown);
    m_Animator.ApplyToSprite(m_Sprite, s_CellSize, s_SpriteSize);
}

void Enemy::Spawn(Entity& target, const sf::Vector2f& spawnPosition, const Terrain& terrain)
{
    auto distSpeed = std::uniform_int_distribution<int>{s_MinSpeed, s_MaxSpeed};
    const auto stats = EnemyStats{
        .maxHealth = s_MaxHealth,
        .moveSpeed = static_cast<float>(distSpeed(s_RandomDevice)),
        .attackDamage = config::enemyAttackDamage,
    };
    Spawn(target, spawnPosition, terrain, stats);
}

void Enemy::Spawn(Entity& target, const sf::Vector2f& spawnPosition, const Terrain& terrain, const EnemyStats& stats)
{
    m_Target = &target;
    m_Terrain = &terrain;
    m_MaxHealth = std::max(1.0f, stats.maxHealth);
    m_Health = m_MaxHealth;
    m_Speed = std::max(0.0f, stats.moveSpeed);
    m_AttackDamage = std::max(0.0f, stats.attackDamage);
    m_AttackTimer = s_AttackCooldown;

    m_Sprite.setPosition(spawnPosition);

    m_CurrentState = EnemyState::FlyDown;
    m_Animator.Play(EnemyState::FlyDown);
    m_Animator.ApplyToSprite(m_Sprite, s_CellSize, s_SpriteSize);
}

auto Enemy::GetFeetBounds() const -> sf::FloatRect
{
    const auto bounds = GetBounds();
    return {
        {bounds.position.x + s_FeetInsetX, bounds.position.y + bounds.size.y - s_FeetHeight},
        {bounds.size.x - (s_FeetInsetX * 2.f), s_FeetHeight}};
}

void Enemy::SetSwarm(const SpatialGrid* swarm)
{
    m_Swarm = swarm;
}

auto Enemy::CalculateSeparation() const -> sf::Vector2f
{
    if (!m_Swarm)
    {
        return {0.f, 0.f};
    }

    auto separation = sf::Vector2f{0.f, 0.f};
    const auto currentPos = GetPos();

    m_Swarm->ForEachNear(
        currentPos,
        s_SeparationRadius,
        [&](const sf::Vector2f& peerPos)
        {
            const auto diff = currentPos - peerPos;
            const auto distance = diff.length();

            if (distance > 0.001f && distance < s_SeparationRadius)
            {
                separation += (diff / distance) * (1.f - (distance / s_SeparationRadius));
            }
        });

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

    const auto toTarget = m_Target->GetPos() - GetPos();
    const auto chaseDirection = toTarget.length() > 0.001f ? toTarget.normalized() : sf::Vector2f{0.f, 0.f};

    const auto separationForce = CalculateSeparation();

    m_Direction = chaseDirection + (separationForce * s_SeparationWeight);

    if (m_Direction.length() > 0.001f)
    {
        if (std::abs(m_Direction.x) > std::abs(m_Direction.y))
        {
            m_CurrentState = (m_Direction.x > 0.f) ? EnemyState::FlyLeft : EnemyState::FlyRight;
        }
        else
        {
            m_CurrentState = (m_Direction.y > 0.f) ? EnemyState::FlyDown : EnemyState::FlyUp;
        }

        auto movement = m_Direction.normalized() * (dt * m_Speed);
        if (m_Terrain)
        {
            movement = m_Terrain->ResolveMovement(GetFeetBounds(), movement);
        }
        m_Sprite.move(movement);
    }

    m_Animator.Play(m_CurrentState);
    m_Animator.Update(dt);
    m_Animator.ApplyToSprite(m_Sprite, s_CellSize, s_SpriteSize);

    m_AttackTimer += dt;
    if (m_AttackTimer >= s_AttackCooldown && GetBounds().findIntersection(m_Target->GetBounds()))
    {
        m_Target->TakeDamage(m_AttackDamage);
        m_AttackTimer = 0.0f;
    }
}

void Enemy::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_Sprite, states);
}
}
