#pragma once

#include "entity.hpp"
#include <SFML/Graphics.hpp>

namespace horde
{
class StaticEntity : public Entity
{
public:
    StaticEntity(float posX, float posY, float texX, float texY, const sf::Texture& texture);
    ~StaticEntity() override = default;

public:
    [[nodiscard]] auto GetSprite() -> sf::Sprite&;
    [[nodiscard]] auto GetSprite() const -> const sf::Sprite&;
    [[nodiscard]] auto GetPos() const -> sf::Vector2f override;
    [[nodiscard]] auto GetBounds() const -> sf::FloatRect override;

private:
    float m_PosX, m_PosY, m_TexX, m_TexY;
    sf::Sprite m_Sprite;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
}
