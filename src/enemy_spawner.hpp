#pragma once

#include "object_pool.hpp"

#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <memory>
#include <random>

namespace horde
{
class Entity;
class Terrain;

class EnemySpawner
{
public:
    EnemySpawner();
    ~EnemySpawner() = default;

    void SpawnWave(ObjectPool<std::unique_ptr<Entity>>& pool,
                   Entity& target,
                   const Terrain& terrain);

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
}
