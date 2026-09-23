#pragma once

#include "entity.hpp"

#include <random>

class Terrain;

class Enemy : public Entity
{
public:
    Enemy(Entity& target);
    ~Enemy() = default;

    auto GetPos() const -> sf::Vector2f override;
    void OnUpdate(float dt) override;
    auto GetBounds() const -> sf::FloatRect override;
    auto GetFeetBounds() const -> sf::FloatRect;
    void SetTerrain(const Terrain& terrain);
    void SetSwarm(const std::vector<std::unique_ptr<Entity>>* swarm);
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
    const std::vector<std::unique_ptr<Entity>>* m_Swarm = nullptr;
    sf::Vector2f m_Direction;
    float timer = 0.0f;

    inline static std::mt19937 s_RandomDevice;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
