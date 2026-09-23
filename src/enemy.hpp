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

private:
    sf::RectangleShape m_Sprite;
    float m_Speed = 100.f;
    Entity* m_Target = nullptr;
    const Terrain* m_Terrain = nullptr;
    sf::Vector2f m_Direction;
    float timer = 0.0f;

    inline static std::mt19937 s_RandomDevice;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
