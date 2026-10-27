#include "player.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "global_config.hpp"
#include "terrain.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace horde
{
Player::Player()
    : m_HealthPoints(100.f)
{
    if (!m_SpriteSheet.loadFromFile(RESOURCES_PATH "player/walk.png"))
    {
        throw std::runtime_error("Failed to load player texture: " RESOURCES_PATH "player/walk.png");
    }
    m_Sprite = std::make_unique<sf::Sprite>(m_SpriteSheet);

    m_Sprite->setOrigin({32.f, 32.f});
    m_Sprite->setScale({0.8f, 0.8f});
    m_Sprite->setPosition(config::playerInitialPosition);

    m_Animator.AddState(PlayerState::IdleUp, {0, 0, 1, 0.2f, false});
    m_Animator.AddState(PlayerState::WalkUp, {0, 1, 8, 0.1f, true});
    m_Animator.AddState(PlayerState::IdleLeft, {1, 0, 1, 0.2f, false});
    m_Animator.AddState(PlayerState::WalkLeft, {1, 1, 8, 0.1f, true});
    m_Animator.AddState(PlayerState::IdleDown, {2, 0, 1, 0.2f, false});
    m_Animator.AddState(PlayerState::WalkDown, {2, 1, 8, 0.1f, true});
    m_Animator.AddState(PlayerState::IdleRight, {3, 0, 1, 0.2f, false});
    m_Animator.AddState(PlayerState::WalkRight, {3, 1, 8, 0.1f, true});

    m_Animator.Play(PlayerState::IdleDown);
}

void Player::SetTerrain(const Terrain& terrain)
{
    m_Terrain = &terrain;
}

void Player::SetPosition(const sf::Vector2f& position)
{
    m_Sprite->setPosition(position);
}

void Player::SetInputEnabled(bool enabled)
{
    m_InputEnabled = enabled;
}

auto Player::GetBounds() const -> sf::FloatRect
{
    const auto pos = m_Sprite->getPosition();
    return {sf::Vector2f(pos.x - 16.f, pos.y - 16.f), sf::Vector2f(32.f, 32.f)};
}

auto Player::GetFeetBounds() const -> sf::FloatRect
{
    return {m_Sprite->getPosition() + sf::Vector2f(-5.f, 8.f), {10.f, 6.f}};
}

auto Player::GetPos() const -> sf::Vector2f
{
    return m_Sprite->getPosition();
}

auto Player::GetHealthPoints() const -> float
{
    return m_HealthPoints;
}

auto Player::GetMaxHealthPoints() const -> float
{
    return m_MaxHealthPoints;
}

void Player::SetImmortal(bool immortal)
{
    m_IsImmortal = immortal;
}

auto Player::IsImmortal() const -> bool
{
    return m_IsImmortal;
}

void Player::SetHealthPoints(float hp)
{
    m_HealthPoints = std::clamp(hp, 0.0f, m_MaxHealthPoints);
}

void Player::TakeDamage(float amount)
{
    if (m_IsImmortal)
    {
        return;
    }
    m_HealthPoints = std::max(0.0f, m_HealthPoints - amount);
}

auto Player::IsDead() const -> bool
{
    if (m_IsImmortal)
    {
        return false;
    }
    return m_HealthPoints <= 0.0f;
}

auto Player::CheckForCollision(const Entity* target) const -> bool
{
    if (target && GetBounds().findIntersection(target->GetBounds()))
    {
        return true;
    }
    return false;
}

void Player::OnUpdate(float dt)
{
    auto direction = sf::Vector2f{0.0f, 0.0f};
    auto isMoving = false;

    const auto isPressed = [this](sf::Keyboard::Key key) { return m_InputEnabled && sf::Keyboard::isKeyPressed(key); };

    if (isPressed(sf::Keyboard::Key::A))
    {
        direction.x += -1.0f;
        m_CurrentState = PlayerState::WalkLeft;
        isMoving = true;
    }
    else if (isPressed(sf::Keyboard::Key::D))
    {
        direction.x += 1.0f;
        m_CurrentState = PlayerState::WalkRight;
        isMoving = true;
    }

    if (isPressed(sf::Keyboard::Key::W))
    {
        direction.y += -1.0f;
        m_CurrentState = PlayerState::WalkUp;
        isMoving = true;
    }
    else if (isPressed(sf::Keyboard::Key::S))
    {
        direction.y += 1.0f;
        m_CurrentState = PlayerState::WalkDown;
        isMoving = true;
    }

    if (!isMoving)
    {
        switch (m_CurrentState)
        {
            case PlayerState::WalkUp: m_CurrentState = PlayerState::IdleUp; break;
            case PlayerState::WalkLeft: m_CurrentState = PlayerState::IdleLeft; break;
            case PlayerState::WalkDown: m_CurrentState = PlayerState::IdleDown; break;
            case PlayerState::WalkRight: m_CurrentState = PlayerState::IdleRight; break;
            default: break;
        }
    }

    m_Animator.Play(m_CurrentState);
    m_Animator.Update(dt);
    m_Animator.ApplyToSprite(*m_Sprite, {64, 64}, {64, 64});

    auto movement = (direction.length() <= 1 ? direction : direction.normalized()) * m_Speed * dt;
    if (m_Terrain) movement = m_Terrain->ResolveMovement(GetFeetBounds(), movement);
    if (m_Terrain)
    {
        const auto currentPosition = m_Sprite->getPosition();
        const auto spriteSize = m_Sprite->getGlobalBounds().size;
        const auto halfSize = spriteSize / 2.f;
        const auto terrainSize = m_Terrain->GetSize();
        const auto nextPosition = currentPosition + movement;
        const auto minX = std::min(halfSize.x, terrainSize.x / 2.f);
        const auto minY = std::min(halfSize.y, terrainSize.y / 2.f);
        const auto maxX = std::max(minX, terrainSize.x - halfSize.x);
        const auto maxY = std::max(minY, terrainSize.y - halfSize.y);
        const auto clampedPosition = sf::Vector2f{
            std::clamp(nextPosition.x, minX, maxX),
            std::clamp(nextPosition.y, minY, maxY)};
        movement = clampedPosition - currentPosition;
    }
    m_Sprite->move(movement);
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(*m_Sprite, states);
}
}
