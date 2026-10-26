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

void EnemySpawner::StartWave(const WaveData& wave, uint32_t waveIndex)
{
    m_WaveIndex = waveIndex;
    m_WaveTimer = 0.0f;
    m_ActiveSpawns.clear();
    m_ActiveSpawns.reserve(wave.spawns.size());

    const auto preDelay = std::max(0.0f, wave.preWaveDelay);
    for (const auto& spawn : wave.spawns)
    {
        if (spawn.count == 0)
        {
            continue;
        }

        m_ActiveSpawns.push_back(ActiveSpawnEntry{
            .data = spawn,
            .spawnedCount = 0,
            .nextSpawnTime = preDelay + std::max(0.0f, spawn.spawnDelay),
        });
    }

    m_HasActiveWave = !m_ActiveSpawns.empty();
}

auto EnemySpawner::IsSpawningComplete() const -> bool
{
    return !m_HasActiveWave;
}

auto EnemySpawner::IsWaveFinished(size_t activeEnemyCount) const -> bool
{
    return !m_HasActiveWave && activeEnemyCount == 0;
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
    m_ActiveSpawns.clear();
    m_WaveTimer = 0.0f;
    m_HasActiveWave = false;
}

auto EnemySpawner::ResolveSpawnPosition(
    const EnemySpawnData& spawnData,
    const sf::Vector2f& targetPos,
    const Terrain& terrain) -> sf::Vector2f
{
    if (spawnData.locationMode == SpawnLocationMode::AroundPlayer)
    {
        return GenerateSpawnPosition(targetPos, terrain);
    }

    constexpr auto halfSize = 16.f;
    constexpr auto margin = 20.f;
    const auto worldSize = terrain.GetSize();

    const auto minX = halfSize + margin;
    const auto maxX = std::max(minX, worldSize.x - halfSize - margin);
    const auto minY = halfSize + margin;
    const auto maxY = std::max(minY, worldSize.y - halfSize - margin);

    auto pos = spawnData.position;
    if (spawnData.count > 1)
    {
        auto jitterDist = std::uniform_real_distribution<float>{-8.0f, 8.0f};
        pos.x += jitterDist(m_Rng);
        pos.y += jitterDist(m_Rng);
    }

    return {
        std::clamp(pos.x, minX, maxX),
        std::clamp(pos.y, minY, maxY),
    };
}

auto EnemySpawner::GenerateSpawnPosition(const sf::Vector2f& center, const Terrain& terrain) -> sf::Vector2f
{
    auto angleDist = std::uniform_real_distribution<float>{0.f, 2.f * std::numbers::pi_v<float>};
    auto radiusDist = std::uniform_real_distribution<float>{config::minSpawnDistance, config::maxSpawnDistance};

    // Enemy sprites are centred on their position (origin 16,16)
    constexpr auto halfSize = 16.f;
    constexpr auto margin = 20.f;
    const auto worldSize = terrain.GetSize();

    const auto minX = halfSize + margin;
    const auto maxX = worldSize.x - halfSize - margin;
    const auto minY = halfSize + margin;
    const auto maxY = worldSize.y - halfSize - margin;

    const auto isFree = [&](const sf::Vector2f& pos)
    {
        const auto feet = sf::FloatRect{
            {pos.x - halfSize + Enemy::s_FeetInsetX, pos.y + halfSize - Enemy::s_FeetHeight},
            {(2.f * halfSize) - (Enemy::s_FeetInsetX * 2.f), Enemy::s_FeetHeight}};
        return !terrain.IsBlocked(feet);
    };

    for (auto attempt = 0; attempt < 50; ++attempt)
    {
        const auto angle = angleDist(m_Rng);
        const auto radius = radiusDist(m_Rng);
        const auto candidate = sf::Vector2f{
            center.x + radius * std::cos(angle),
            center.y + radius * std::sin(angle)
        };

        if (candidate.x >= minX && candidate.x <= maxX && candidate.y >= minY && candidate.y <= maxY && isFree(candidate))
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
} // namespace horde
