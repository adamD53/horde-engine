#include "camera.hpp"
#include "global_config.hpp"

Camera::Camera(sf::RenderWindow &window) {
  m_View = window.getDefaultView();
  m_View.zoom(config::cameraZoomLevel);
}

auto Camera::GetViewPoint() const -> sf::View { return m_View; }

auto Camera::GetUIViewPoint() const -> sf::View { return m_UIView; }

void Camera::SetCenterPoint(const sf::Vector2f &centerPoint) {
  const auto viewHalfSize = m_View.getSize() / 2.f;

  const float minX = config::leftBound + viewHalfSize.x;
  const float maxX = config::rightBound - viewHalfSize.x;
  const float minY = config::topBound + viewHalfSize.y;
  const float maxY = config::bottomBound - viewHalfSize.y;

  const float clampedX = std::clamp(centerPoint.x, minX, maxX);
  const float clampedY = std::clamp(centerPoint.y, minY, maxY);

  m_View.setCenter({clampedX, clampedY});
}

void Camera::SetUIViewPoint() {
  m_UIView = sf::View(sf::FloatRect({0.f, 0.f}, m_View.getSize()));
}
