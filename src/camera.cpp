#include "camera.hpp"
#include "global_config.hpp"

#include <algorithm>
#include <cmath>

namespace horde
{
Camera::Camera(sf::RenderWindow& window)
{
    m_View = window.getDefaultView();
    m_View.zoom(config::cameraZoomLevel);
}

auto Camera::GetViewPoint() const -> sf::View
{
    return m_View;
}

auto Camera::GetUIViewPoint() const -> sf::View
{
    return m_UIView;
}

void Camera::SetCenterPoint(const sf::Vector2f& centerPoint)
{
    const auto viewHalfSize = m_View.getSize() / 2.f;

    const auto minX = static_cast<float>(config::leftBound) + viewHalfSize.x;
    const auto maxX = static_cast<float>(config::rightBound) - viewHalfSize.x;
    const auto minY = static_cast<float>(config::topBound) + viewHalfSize.y;
    const auto maxY = static_cast<float>(config::bottomBound) - viewHalfSize.y;

    const auto clampedX = std::clamp(centerPoint.x, minX, maxX);
    const auto clampedY = std::clamp(centerPoint.y, minY, maxY);

    m_View.setCenter({std::round(clampedX), std::round(clampedY)});
}

void Camera::SetUIViewPoint()
{
    m_UIView = sf::View(sf::FloatRect({0.f, 0.f}, m_View.getSize()));
}
}
