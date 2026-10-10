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
    m_CenterPosition = m_View.getCenter();
    SetUIViewPoint();
}

auto Camera::GetViewPoint() const -> sf::View
{
    return m_View;
}

auto Camera::GetUIViewPoint() const -> sf::View
{
    return m_UIView;
}

auto Camera::ClampPosition(const sf::Vector2f& position) const -> sf::Vector2f
{
    const auto viewHalfSize = m_View.getSize() / 2.f;

    const auto minX = static_cast<float>(config::leftBound) + viewHalfSize.x;
    const auto maxX = static_cast<float>(config::rightBound) - viewHalfSize.x;
    const auto minY = static_cast<float>(config::topBound) + viewHalfSize.y;
    const auto maxY = static_cast<float>(config::bottomBound) - viewHalfSize.y;

    const auto clampedX = (minX > maxX) ? static_cast<float>(config::leftBound + config::rightBound) * 0.5f
                                        : std::clamp(position.x, minX, maxX);
    const auto clampedY = (minY > maxY) ? static_cast<float>(config::topBound + config::bottomBound) * 0.5f
                                        : std::clamp(position.y, minY, maxY);

    return {clampedX, clampedY};
}

void Camera::SetCenterPoint(const sf::Vector2f& centerPoint)
{
    m_CenterPosition = ClampPosition(centerPoint);
    m_View.setCenter(m_CenterPosition);
}

void Camera::Update(const sf::Vector2f& targetPosition, float dt)
{
    const auto clampedTarget = ClampPosition(targetPosition);
    const auto factor = 1.0f - std::exp(-config::cameraSmoothSpeed * dt);

    m_CenterPosition.x = std::lerp(m_CenterPosition.x, clampedTarget.x, factor);
    m_CenterPosition.y = std::lerp(m_CenterPosition.y, clampedTarget.y, factor);

    m_View.setCenter(m_CenterPosition);
}

void Camera::SetUIViewPoint()
{
    m_UIView = sf::View(sf::FloatRect({0.f, 0.f}, m_View.getSize()));
}
}
