#pragma once

#include "entity.hpp"
#include "global_config.hpp"
#include "object_pool.hpp"

#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cstdint>
#include <random>
#include <variant>

namespace horde
{
class Terrain;

class EnemySpawner
{
public:
    EnemySpawner();
    ~EnemySpawner() = default;

    template <typename Pool>
    void SpawnWave(Pool& pool, Entity& target, const Terrain& terrain);

    auto GetCurrentWaveCount() const -> uint32_t;
    auto GetWaveIndex() const -> uint32_t;
    void Reset();

private:
    [[nodiscard]] auto GenerateSpawnPosition(const sf::Vector2f& center) -> sf::Vector2f;

private:
    uint32_t m_CurrentWaveCount;
    uint32_t m_WaveIndex{0};
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

        const auto spawnPos = GenerateSpawnPosition(targetPos);
        std::visit([&](auto& enemy) {
            enemy.Spawn(target, spawnPos, terrain);
        }, *entityVariant);
    }

    ++m_WaveIndex;
    m_CurrentWaveCount = std::min(m_CurrentWaveCount * 2, config::maxEnemyCount);
}
}
