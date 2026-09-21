#include "collision_grid.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

void CollisionGrid::Reset(unsigned width, unsigned height, sf::Vector2f tileSize)
{
    if (!width || !height || tileSize.x <= 0 || tileSize.y <= 0)
        throw std::runtime_error("Invalid collision grid dimensions");
    m_Width = width;
    m_Height = height;
    m_TileSize = tileSize;
    m_Cells.assign(static_cast<size_t>(width) * height, {});
}

void CollisionGrid::Add(unsigned x, unsigned y, sf::FloatRect bounds)
{
    bounds.position += sf::Vector2f(x * m_TileSize.x, y * m_TileSize.y);
    m_Cells.at(static_cast<size_t>(y) * m_Width + x).push_back(bounds);
}

auto CollisionGrid::IsBlocked(sf::FloatRect body) const -> bool
{
    if (m_Cells.empty() || body.position.x < 0 || body.position.y < 0 ||
        body.position.x + body.size.x > m_Width * m_TileSize.x ||
        body.position.y + body.size.y > m_Height * m_TileSize.y) return true;
    const auto left = static_cast<unsigned>(body.position.x / m_TileSize.x);
    const auto top = static_cast<unsigned>(body.position.y / m_TileSize.y);
    const auto right = std::min(m_Width - 1, static_cast<unsigned>((body.position.x + body.size.x) / m_TileSize.x));
    const auto bottom = std::min(m_Height - 1, static_cast<unsigned>((body.position.y + body.size.y) / m_TileSize.y));
    for (auto y = top; y <= bottom; ++y)
        for (auto x = left; x <= right; ++x)
            for (const auto& obstacle : m_Cells[static_cast<size_t>(y) * m_Width + x])
                if (body.findIntersection(obstacle)) return true;
    return false;
}

auto CollisionGrid::Move(sf::FloatRect body, sf::Vector2f delta) const -> sf::Vector2f
{
    if (m_Cells.empty()) return delta;
    auto result = sf::Vector2f{};
    result.x = Sweep(body, delta.x, true);
    body.position.x += result.x;
    result.y = Sweep(body, delta.y, false);
    return result;
}

auto CollisionGrid::Sweep(const sf::FloatRect& body, float distance, bool horizontal) const -> float
{
    const auto start = horizontal ? body.position.x : body.position.y;
    const auto extent = horizontal ? body.size.x : body.size.y;
    const auto worldEnd = horizontal ? m_Width * m_TileSize.x : m_Height * m_TileSize.y;
    distance = std::clamp(distance, std::min(0.f, -start), std::max(0.f, worldEnd - start - extent));
    auto end = body.position;
    (horizontal ? end.x : end.y) += distance;
    const auto left = std::clamp(static_cast<int>(std::floor(std::min(body.position.x, end.x) / m_TileSize.x)), 0, static_cast<int>(m_Width) - 1);
    const auto top = std::clamp(static_cast<int>(std::floor(std::min(body.position.y, end.y) / m_TileSize.y)), 0, static_cast<int>(m_Height) - 1);
    const auto right = std::clamp(static_cast<int>(std::floor((std::max(body.position.x, end.x) + body.size.x) / m_TileSize.x)), 0, static_cast<int>(m_Width) - 1);
    const auto bottom = std::clamp(static_cast<int>(std::floor((std::max(body.position.y, end.y) + body.size.y) / m_TileSize.y)), 0, static_cast<int>(m_Height) - 1);
    for (auto y = top; y <= bottom; ++y)
        for (auto x = left; x <= right; ++x)
            for (const auto& obstacle : m_Cells[static_cast<size_t>(y) * m_Width + x])
            {
                const auto crossStart = horizontal ? body.position.y : body.position.x;
                const auto crossSize = horizontal ? body.size.y : body.size.x;
                const auto obstacleCross = horizontal ? obstacle.position.y : obstacle.position.x;
                const auto obstacleCrossSize = horizontal ? obstacle.size.y : obstacle.size.x;
                if (crossStart >= obstacleCross + obstacleCrossSize || crossStart + crossSize <= obstacleCross)
                    continue;
                const auto nearEdge = horizontal ? obstacle.position.x : obstacle.position.y;
                const auto farEdge = nearEdge + (horizontal ? obstacle.size.x : obstacle.size.y);
                if (distance > 0 && start + extent <= nearEdge)
                    distance = std::min(distance, nearEdge - start - extent);
                else if (distance < 0 && start >= farEdge)
                    distance = std::max(distance, farEdge - start);
            }
    return distance;
}
