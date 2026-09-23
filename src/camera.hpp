#pragma once

#include <SFML/Graphics.hpp>

class Camera
{
public:
    Camera(sf::RenderWindow& window);

    auto GetViewPoint() const -> sf::View;
    auto GetUIViewPoint() const -> sf::View;
    
    void SetUIViewPoint();
    void SetCenterPoint(const sf::Vector2f& centerPoint);

private:
    sf::View m_View, m_UIView;
};
