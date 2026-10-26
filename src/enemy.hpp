#pragma once

#include "animation.hpp"
#include "entity.hpp"
#include "global_config.hpp"
#include "level_data.hpp"
#include "spatial_grid.hpp"

#include <random>
#include <span>

namespace horde
{
class Terrain;

enum class EnemyState
{
    FlyUp,
    FlyRight,
    FlyDown,
    FlyLeft,
    Count
};

class Enemy : public Entity
{
public:
    static constexpr float s_MaxHealth = 100.f;
    static constexpr float s_AttackCooldown = 1.0f;
    static constexpr int s_MinSpeed = 50;
    static constexpr int s_MaxSpeed = 100;
    static constexpr float s_SeparationRadius = 45.f;
    static constexpr float s_SeparationWeight = 1.5f;

    static constexpr sf::Vector2i s_CellSize{32, 32};
    static constexpr sf::Vector2i s_SpriteSize{32, 32};
    static constexpr sf::Vector2f s_SpriteOrigin{16.f, 16.f};
    static constexpr uint32_t s_FrameCount = 3;
    static constexpr float s_FrameDuration = 0.12f;

    static constexpr sf::Vector2f s_HpBarSize{28.f, 4.f};
    static constexpr float s_HpBarOffsetY = 7.f;
    static constexpr float s_HpBarOutlineThickness = 1.f;
    static constexpr sf::Color s_HpBackgroundColor{20, 20, 20};

    static constexpr float s_FeetInsetX = 4.f;
    static constexpr float s_FeetHeight = 10.f;

public:
    Enemy();
    ~Enemy() override = default;
    Enemy(Enemy&&) noexcept = default;
    Enemy& operator=(Enemy&&) noexcept = default;
    Enemy(const Enemy&) = default;
    Enemy& operator=(const Enemy&) = default;

    void Spawn(Entity& target, const sf::Vector2f& spawnPosition, const Terrain& terrain);
    void Spawn(Entity& target, const sf::Vector2f& spawnPosition, const Terrain& terrain, const EnemyStats& stats);
    auto GetPos() const -> sf::Vector2f override;
    void OnUpdate(float dt) override;
    auto GetBounds() const -> sf::FloatRect override;
    auto GetFeetBounds() const -> sf::FloatRect;
    void SetSwarm(const SpatialGrid* swarm);
    void TakeDamage(float amount) override;
    auto IsDead() const -> bool override;
    auto GetHealth() const -> float;
    auto GetMaxHealth() const -> float;

private:
    [[nodiscard]] auto CalculateSeparation() const -> sf::Vector2f;
    void UpdateHpBar();
    static auto GetTexture() -> const sf::Texture&;

private:
    sf::Sprite m_Sprite;
    Animator<EnemyState, static_cast<size_t>(EnemyState::Count)> m_Animator;
    EnemyState m_CurrentState = EnemyState::FlyDown;
    sf::RectangleShape m_HpBackgroundBar;
    sf::RectangleShape m_HpBar;
    float m_Speed = 100.f;
    float m_MaxHealth = s_MaxHealth;
    float m_Health = s_MaxHealth;
    float m_AttackDamage = config::enemyAttackDamage;
    Entity* m_Target = nullptr;
    const Terrain* m_Terrain = nullptr;
    const SpatialGrid* m_Swarm = nullptr;
    sf::Vector2f m_Direction;
    float m_AttackTimer = s_AttackCooldown;

    inline static std::mt19937 s_RandomDevice{std::random_device{}()};

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
}
