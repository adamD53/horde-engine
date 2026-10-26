#pragma once

#include "entity.hpp"
#include "global_config.hpp"
#include "level_data.hpp"
#include "object_pool.hpp"

#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <variant>
#include <vector>

namespace horde
{
class Terrain;

struct ActiveSpawnEntry
{
    EnemySpawnData data{};
    uint32_t spawnedCount{0};
    float nextSpawnTime{0.0f};
};

class EnemySpawner
{
public:
    EnemySpawner();
    ~EnemySpawner() = default;

    template <typename Pool>
    void SpawnWave(Pool& pool, Entity& target, const Terrain& terrain);

    void StartWave(const WaveData& wave, uint32_t waveIndex);

    template <typename Pool>
    void Update(float dt, Pool& pool, Entity& target, const Terrain& terrain);

    [[nodiscard]] auto IsSpawningComplete() const -> bool;
    [[nodiscard]] auto IsWaveFinished(size_t activeEnemyCount) const -> bool;
    [[nodiscard]] auto GetCurrentWaveCount() const -> uint32_t;
    [[nodiscard]] auto GetWaveIndex() const -> uint32_t;
    void Reset();

private:
    [[nodiscard]] auto GenerateSpawnPosition(const sf::Vector2f& center, const Terrain& terrain) -> sf::Vector2f;
    [[nodiscard]] auto ResolveSpawnPosition(
        const EnemySpawnData& spawnData,
        const sf::Vector2f& targetPos,
        const Terrain& terrain) -> sf::Vector2f;

private:
    uint32_t m_CurrentWaveCount;
    uint32_t m_WaveIndex{0};
    std::vector<ActiveSpawnEntry> m_ActiveSpawns{};
    float m_WaveTimer{0.0f};
    bool m_HasActiveWave{false};
    std::mt19937 m_Rng{std::random_device{}()};
};

template <typename Pool>
void EnemySpawner::SpawnWave(Pool& pool, Entity& target, const Terrain& terrain)
{
    const auto targetPos = target.GetPos();

    for (auto i = 0u; i < m_CurrentWaveCount; ++i)
    {
        auto* entityVariant = pool.Acquire();
        if (!entityVariant)
        {
            break;
        }

        const auto spawnPos = GenerateSpawnPosition(targetPos, terrain);
        std::visit([&](auto& enemy) {
            enemy.Spawn(target, spawnPos, terrain);
        }, *entityVariant);
    }

    ++m_WaveIndex;
    m_CurrentWaveCount = std::min(m_CurrentWaveCount * 2, config::maxEnemyCount);
    m_ActiveSpawns.clear();
    m_HasActiveWave = false;
}

template <typename Pool>
void EnemySpawner::Update(float dt, Pool& pool, Entity& target, const Terrain& terrain)
{
    if (!m_HasActiveWave)
    {
        return;
    }

    m_WaveTimer += dt;
    const auto targetPos = target.GetPos();
    auto allComplete = true;

    for (auto& entry : m_ActiveSpawns)
    {
        while (entry.spawnedCount < entry.data.count && m_WaveTimer >= entry.nextSpawnTime)
        {
            auto* entityVariant = pool.Acquire();
            if (!entityVariant)
            {
                break;
            }

            const auto spawnPos = ResolveSpawnPosition(entry.data, targetPos, terrain);
            std::visit(
                [&](auto& enemy)
                {
                    enemy.Spawn(target, spawnPos, terrain, entry.data.stats);
                },
                *entityVariant);

            ++entry.spawnedCount;
            entry.nextSpawnTime += std::max(0.0f, entry.data.spawnInterval);
        }

        if (entry.spawnedCount < entry.data.count)
        {
            allComplete = false;
        }
    }

    if (allComplete)
    {
        m_HasActiveWave = false;
    }
}
} // namespace horde
