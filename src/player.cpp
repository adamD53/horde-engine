#include "player.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "global_config.hpp"
#include "terrain.hpp"

#include <algorithm>
#include <cmath>

namespace horde
{
Player::Player()
    : m_HealthPoints(100.f)
{
    m_AnimationTextures.emplace(std::make_pair("walk_down", sf::Texture(RESOURCES_PATH "player/Walk_Down-Sheet.png")));
    m_AnimationTextures.emplace(std::make_pair("walk_up", sf::Texture(RESOURCES_PATH "player/Walk_Up-Sheet.png")));
    m_AnimationTextures.emplace(std::make_pair("walk_side", sf::Texture(RESOURCES_PATH "player/Walk_Side-Sheet.png")));
    m_AnimationTextures.emplace(std::make_pair("idle_down", sf::Texture(RESOURCES_PATH "player/Idle_Down-Sheet.png")));

    m_Sprite = std::make_unique<sf::Sprite>(m_AnimationTextures["idle_down"]);
    const auto textureHeight = m_Sprite->getTextureRect().size.y;
    m_Sprite->setTextureRect(sf::IntRect({16, 16}, {32, 32}));
    // Measured pivot keeps the artwork aligned when flipping horizontally.
    m_Sprite->setOrigin({16 / 2.f, textureHeight / 2.f});
    m_Sprite->setPosition(config::playerInitialPosition);

    m_Animation = std::make_unique<Animation>(0.2f);
}

void Player::SetTerrain(const Terrain& terrain)
{
    m_Terrain = &terrain;
}

auto Player::GetBounds() const -> sf::FloatRect
{
    return m_Sprite->getGlobalBounds();
}

auto Player::GetFeetBounds() const -> sf::FloatRect
{
    // Anchored to the feet, independent of animation frames and facing direction.
    return {m_Sprite->getPosition() + sf::Vector2f(-5.f, -6.f), {10.f, 6.f}};
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

void Player::TakeDamage(float amount)
{
    m_HealthPoints = std::max(0.0f, m_HealthPoints - amount);
}

auto Player::IsDead() const -> bool
{
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
    auto currentAnimationFrameIndex = uint32_t{0};
    auto animationRow = 0;

    const auto currentPos = GetPos();
    m_CurrentState = PlayerState::IdleDown;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) && (currentPos.x > config::leftBound))
    {
        direction.x += -1.0f;
        m_CurrentState = PlayerState::WalkLeft;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) && (currentPos.x < config::rightBound))
    {
        direction.x += 1.0f;
        m_CurrentState = PlayerState::WalkRight;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) && (currentPos.y > config::topBound))
    {
        direction.y += -1.0f;
        m_CurrentState = PlayerState::WalkUp;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) && (currentPos.y < config::bottomBound))
    {
        direction.y += 1.0f;
        m_CurrentState = PlayerState::WalkDown;
    }

    const auto framesCount = (m_CurrentState == PlayerState::IdleDown) ? 4u : 6u;
    currentAnimationFrameIndex = m_Animation->Update(framesCount);
    switch (m_CurrentState)
    {
        case PlayerState::IdleDown:
            m_Sprite->setTexture(m_AnimationTextures["idle_down"]);
            animationRow = 16;
            break;
        case PlayerState::WalkUp:
            m_Sprite->setTexture(m_AnimationTextures["walk_up"]);
            animationRow = 16;
            break;
        case PlayerState::WalkDown:
            m_Sprite->setTexture(m_AnimationTextures["walk_down"]);
            animationRow = 16;
            break;
        case PlayerState::WalkRight:
            m_FacingRight = true;
            m_Sprite->setTexture(m_AnimationTextures["walk_side"]);
            animationRow = 16;
            break;
        case PlayerState::WalkLeft:
            m_FacingRight = false;
            m_Sprite->setTexture(m_AnimationTextures["walk_side"]);
            animationRow = 16;
            break;
    }

    const auto frameX = static_cast<int>(currentAnimationFrameIndex * m_TexStride);
    m_Sprite->setTextureRect(
        sf::IntRect({frameX, animationRow}, {config::playerTextureWidth, config::playerTextureHeight}));

    auto movement = (direction.length() <= 1 ? direction : direction.normalized()) * m_Speed * dt;
    if (m_Terrain) movement = m_Terrain->ResolveMovement(GetFeetBounds(), movement);
    m_Sprite->move(movement);
    m_Sprite->setScale({(m_FacingRight ? 1.0f : -1.0f), 1.0f});
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    const auto pos = m_Sprite->getPosition();
    const auto offset = sf::Vector2f{std::round(pos.x) - pos.x, std::round(pos.y) - pos.y};

    auto snappedStates = states;
    snappedStates.transform.translate(offset);

    target.draw(*m_Sprite, snappedStates);
}
}

