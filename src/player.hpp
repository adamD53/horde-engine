#pragma once

#include "animation.hpp"
#include "entity.hpp"

#include <memory>
#include <unordered_map>

namespace horde
{
enum class PlayerState
{
    IdleUp,
    WalkUp,
    IdleLeft,
    WalkLeft,
    IdleDown,
    WalkDown,
    IdleRight,
    WalkRight,
    Count
};

class Terrain;

class Player : public Entity
{
public:
    Player();
    ~Player() = default;

    void OnUpdate(float dt) override;
    void SetTerrain(const Terrain& terrain);
    auto GetFeetBounds() const -> sf::FloatRect;
    auto GetPos() const -> sf::Vector2f override;
    auto GetHealthPoints() const -> float;
    auto GetMaxHealthPoints() const -> float;
    void TakeDamage(float amount) override;
    auto IsDead() const -> bool override;
    auto GetBounds() const -> sf::FloatRect override;
    auto CheckForCollision(const Entity* target) const -> bool;

private:
    sf::Texture m_SpriteSheet;
    sf::Texture m_WeaponTexture;
    std::unique_ptr<sf::Sprite> m_Sprite;
    std::unique_ptr<sf::Sprite> m_WeaponSprite;
    Animator<PlayerState, static_cast<size_t>(PlayerState::Count)> m_Animator;
    PlayerState m_CurrentState = PlayerState::IdleDown;
    const float m_Speed = 150.0f;
    const float m_MaxHealthPoints = 100.0f;
    float m_HealthPoints = 100.0f;
    const Terrain* m_Terrain = nullptr;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
}
