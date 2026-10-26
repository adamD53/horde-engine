#pragma once

#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace horde
{
enum class EnemyType : uint8_t
{
    Bat = 0,
    Count
};

enum class SpawnLocationMode : uint8_t
{
    WorldPosition = 0,
    AroundPlayer = 1
};

struct EnemyStats
{
    float maxHealth{100.0f};
    float moveSpeed{75.0f};
    float attackDamage{10.0f};
};

struct EnemySpawnData
{
    EnemyType type{EnemyType::Bat};
    SpawnLocationMode locationMode{SpawnLocationMode::WorldPosition};
    sf::Vector2f position{500.0f, 300.0f};
    uint32_t count{1};
    float spawnDelay{0.0f};
    float spawnInterval{0.2f};
    EnemyStats stats{};
};

struct WaveData
{
    std::string name{"Wave 1"};
    float preWaveDelay{1.0f};
    std::vector<EnemySpawnData> spawns{};
};

struct LevelData
{
    std::string name{"Default Level"};
    std::string mapFile{"world/map.tmx"};
    sf::Vector2f playerSpawn{536.0f, 300.0f};
    std::vector<WaveData> waves{};
};
} // namespace horde
