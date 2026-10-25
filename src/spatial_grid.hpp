#pragma once

#include <SFML/System/Vector2.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <span>
#include <vector>

namespace horde
{
class SpatialGrid
{
public:
    void Configure(const sf::Vector2f& worldSize, float cellSize)
    {
        m_CellSize = cellSize;
        m_Columns = std::max(1, static_cast<int>(std::ceil(worldSize.x / cellSize)));
        m_Rows = std::max(1, static_cast<int>(std::ceil(worldSize.y / cellSize)));
    }

    void Build(std::span<const sf::Vector2f> points)
    {
        m_Points = points;
        m_CellStart.assign(static_cast<size_t>(m_Columns) * m_Rows + 1, 0);
        m_Sorted.resize(points.size());

        for (const auto& point : points)
        {
            ++m_CellStart[CellIndex(point) + 1];
        }
        for (size_t i = 1; i < m_CellStart.size(); ++i)
        {
            m_CellStart[i] += m_CellStart[i - 1];
        }

        m_Fill.assign(m_CellStart.begin(), m_CellStart.end() - 1);
        for (size_t i = 0; i < points.size(); ++i)
        {
            m_Sorted[m_Fill[CellIndex(points[i])]++] = static_cast<uint32_t>(i);
        }
    }

    template <typename Fn>
    void ForEachNear(const sf::Vector2f& center, float radius, Fn&& fn) const
    {
        if (m_CellStart.empty()) return;

        const auto minCol = ClampCol(static_cast<int>(std::floor((center.x - radius) / m_CellSize)));
        const auto maxCol = ClampCol(static_cast<int>(std::floor((center.x + radius) / m_CellSize)));
        const auto minRow = ClampRow(static_cast<int>(std::floor((center.y - radius) / m_CellSize)));
        const auto maxRow = ClampRow(static_cast<int>(std::floor((center.y + radius) / m_CellSize)));

        for (auto row = minRow; row <= maxRow; ++row)
        {
            for (auto col = minCol; col <= maxCol; ++col)
            {
                const auto cell = static_cast<size_t>(row) * m_Columns + col;
                for (auto i = m_CellStart[cell]; i < m_CellStart[cell + 1]; ++i)
                {
                    fn(m_Points[m_Sorted[i]]);
                }
            }
        }
    }

private:
    [[nodiscard]] auto ClampCol(int col) const -> int
    {
        return std::clamp(col, 0, m_Columns - 1);
    }
    [[nodiscard]] auto ClampRow(int row) const -> int
    {
        return std::clamp(row, 0, m_Rows - 1);
    }

    [[nodiscard]] auto CellIndex(const sf::Vector2f& point) const -> size_t
    {
        const auto col = ClampCol(static_cast<int>(std::floor(point.x / m_CellSize)));
        const auto row = ClampRow(static_cast<int>(std::floor(point.y / m_CellSize)));
        return static_cast<size_t>(row) * m_Columns + col;
    }

    std::span<const sf::Vector2f> m_Points;
    std::vector<uint32_t> m_CellStart;
    std::vector<uint32_t> m_Sorted;
    std::vector<uint32_t> m_Fill;
    float m_CellSize = 1.f;
    int m_Columns = 1;
    int m_Rows = 1;
};
}
