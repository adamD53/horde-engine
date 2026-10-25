#pragma once

#include <SFML/Graphics.hpp>

namespace horde
{
class Camera
{
public:
    Camera(sf::RenderWindow& window, const sf::Vector2f& worldSize);

    auto GetViewPoint() const -> sf::View;
    auto GetUIViewPoint() const -> sf::View;
    
    void SetUIViewPoint();
    void OnResize(const sf::Vector2u& newSize);
    void SetCenterPoint(const sf::Vector2f& centerPoint);
    void Update(const sf::Vector2f& targetPosition, float dt);

private:
    auto ClampPosition(const sf::Vector2f& position) const -> sf::Vector2f;

    sf::View m_View, m_UIView;
    sf::Vector2f m_CenterPosition;
    sf::Vector2f m_WorldSize;
    sf::Vector2f m_WindowSize;
};
}
