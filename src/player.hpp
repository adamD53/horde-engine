#pragma once

#include "animation.hpp"
#include "entity.hpp"

#include <memory>
#include <unordered_map>

enum class PlayerState
{
    IdleDown,
    WalkUp,
    WalkDown,
    WalkRight,
    WalkLeft
}; // TODO: Add idle animation in all directions

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
    auto GetBounds() const -> sf::FloatRect override;
    auto CheckForCollision(const Entity* target) const -> bool;

private:
    std::unordered_map<std::string, sf::Texture> m_AnimationTextures;
    sf::Texture m_WeaponTexture;
    std::unique_ptr<sf::Sprite> m_Sprite;
    std::unique_ptr<sf::Sprite> m_WeaponSprite;
    std::unique_ptr<Animation> m_Animation;
    PlayerState m_CurrentState = PlayerState::IdleDown;
    bool m_FacingRight = false;
    const uint32_t m_TexStride = 64;
    const float m_Speed = 150.0f;
    float m_HealthPoints;
    const Terrain* m_Terrain = nullptr;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
