#include "enemy_spawner.hpp"
#include "enemy.hpp"
#include "global_config.hpp"
#include "terrain.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace horde
{
EnemySpawner::EnemySpawner()
    : m_CurrentWaveCount(config::initialEnemyCount)
{
}

auto EnemySpawner::GetCurrentWaveCount() const -> uint32_t
{
    return m_CurrentWaveCount;
}

auto EnemySpawner::GetWaveIndex() const -> uint32_t
{
    return m_WaveIndex;
}

void EnemySpawner::Reset()
{
    m_CurrentWaveCount = config::initialEnemyCount;
    m_WaveIndex = 0;
}

auto EnemySpawner::GenerateSpawnPosition(const sf::Vector2f& center) -> sf::Vector2f
{
    auto angleDist = std::uniform_real_distribution<float>{0.f, 2.f * std::numbers::pi_v<float>};
    auto radiusDist = std::uniform_real_distribution<float>{config::minSpawnDistance, config::maxSpawnDistance};

    constexpr auto enemySize = 50.f;
    constexpr auto margin = 20.f;

    const auto minX = static_cast<float>(config::leftBound) + margin;
    const auto maxX = static_cast<float>(config::rightBound) - enemySize - margin;
    const auto minY = static_cast<float>(config::topBound) + margin;
    const auto maxY = static_cast<float>(config::bottomBound) - enemySize - margin;

    for (auto attempt = 0; attempt < 10; ++attempt)
    {
        const auto angle = angleDist(m_Rng);
        const auto radius = radiusDist(m_Rng);
        const auto candidate = sf::Vector2f{
            center.x + radius * std::cos(angle),
            center.y + radius * std::sin(angle)
        };

        if (candidate.x >= minX && candidate.x <= maxX && candidate.y >= minY && candidate.y <= maxY)
        {
            return candidate;
        }
    }

    const auto angle = angleDist(m_Rng);
    const auto radius = radiusDist(m_Rng);
    return {
        std::clamp(center.x + radius * std::cos(angle), minX, maxX),
        std::clamp(center.y + radius * std::sin(angle), minY, maxY)
    };
}
}
