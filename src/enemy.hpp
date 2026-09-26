#pragma once

#include "entity.hpp"

#include <random>
#include <span>

namespace horde
{
class Terrain;

class Enemy : public Entity
{
public:
    Enemy();
    ~Enemy() = default;

    void Spawn(Entity& target, const sf::Vector2f& spawnPosition, const Terrain& terrain);
    auto GetPos() const -> sf::Vector2f override;
    void OnUpdate(float dt) override;
    auto GetBounds() const -> sf::FloatRect override;
    auto GetFeetBounds() const -> sf::FloatRect;
    void SetTerrain(const Terrain& terrain);
    void SetSwarm(std::span<const std::unique_ptr<Entity>> swarm);
    void TakeDamage(float amount) override;
    auto IsDead() const -> bool override;
    auto GetHealth() const -> float;
    auto GetMaxHealth() const -> float;

private:
    [[nodiscard]] auto CalculateSeparation() const -> sf::Vector2f;

private:
    sf::RectangleShape m_Sprite;
    sf::RectangleShape m_HpBackgroundBar;
    sf::RectangleShape m_HpBar;
    float m_Speed = 100.f;
    float m_MaxHealth = 100.f;
    float m_Health = 100.f;
    Entity* m_Target = nullptr;
    const Terrain* m_Terrain = nullptr;
    std::span<const std::unique_ptr<Entity>> m_Swarm;
    sf::Vector2f m_Direction;
    const float m_AttackCooldown = 1.0f;
    float m_AttackTimer = 1.0f;

    inline static std::mt19937 s_RandomDevice;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
}
