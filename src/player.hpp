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
    ~Player() override = default;
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    Player(Player&&) = delete;
    Player& operator=(Player&&) = delete;

    void OnUpdate(float dt) override;
    void SetTerrain(const Terrain& terrain);
    void SetInputEnabled(bool enabled);
    void SetImmortal(bool immortal);
    [[nodiscard]] auto IsImmortal() const -> bool;
    void SetHealthPoints(float hp);
    void TakeDamage(float amount) override;
    auto CheckForCollision(const Entity* target) const -> bool;
    [[nodiscard]] auto GetFeetBounds() const -> sf::FloatRect;
    [[nodiscard]] auto GetPos() const -> sf::Vector2f override;
    [[nodiscard]] auto GetHealthPoints() const -> float;
    [[nodiscard]] auto GetMaxHealthPoints() const -> float;
    [[nodiscard]] auto IsDead() const -> bool override;
    [[nodiscard]] auto GetBounds() const -> sf::FloatRect override;

private:
    sf::Texture m_SpriteSheet;
    std::unique_ptr<sf::Sprite> m_Sprite;
    Animator<PlayerState, static_cast<size_t>(PlayerState::Count)> m_Animator;
    PlayerState m_CurrentState = PlayerState::IdleDown;
    const float m_Speed = 150.0f;
    const float m_MaxHealthPoints = 100.0f;
    float m_HealthPoints = 100.0f;
    const Terrain* m_Terrain = nullptr;
    bool m_InputEnabled = true;
    bool m_IsImmortal = false;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
}
