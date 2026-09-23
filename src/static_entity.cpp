#include "static_entity.hpp"

StaticEntity::StaticEntity(float posX, float posY, float texX, float texY, const sf::Texture* texture)
    : m_PosX(posX)
    , m_PosY(posY)
    , m_TexX(texX)
    , m_TexY(texY)
    , m_Sprite(*texture)
{
    m_Sprite.setTextureRect(sf::IntRect({static_cast<int>(m_TexX), static_cast<int>(m_TexY)}, {16, 16}));
    m_Sprite.setPosition({m_PosX, m_PosY});
}

auto StaticEntity::GetSprite() -> sf::Sprite&
{
    return m_Sprite;
}

auto StaticEntity::GetSprite() const -> const sf::Sprite&
{
    return m_Sprite;
}

auto StaticEntity::GetBounds() const -> sf::FloatRect
{
    return m_Sprite.getGlobalBounds();
}

auto StaticEntity::GetPos() const -> sf::Vector2f
{
    return m_Sprite.getPosition();
}

void StaticEntity::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_Sprite, states);
}
