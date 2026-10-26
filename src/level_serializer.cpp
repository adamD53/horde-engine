#include "level_serializer.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

namespace sf
{
void to_json(nlohmann::json& j, const Vector2f& v)
{
    j = nlohmann::json{{"x", v.x}, {"y", v.y}};
}

void from_json(const nlohmann::json& j, Vector2f& v)
{
    v.x = j.value("x", 0.0f);
    v.y = j.value("y", 0.0f);
}
} // namespace sf

namespace horde
{
NLOHMANN_JSON_SERIALIZE_ENUM(EnemyType, {
    {EnemyType::Bat, "Bat"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(SpawnLocationMode, {
    {SpawnLocationMode::WorldPosition, "WorldPosition"},
    {SpawnLocationMode::AroundPlayer, "AroundPlayer"},
})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EnemyStats, maxHealth, moveSpeed, attackDamage)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    EnemySpawnData,
    type,
    locationMode,
    position,
    count,
    spawnDelay,
    spawnInterval,
    stats)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WaveData, name, preWaveDelay, spawns)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LevelData, name, mapFile, playerSpawn, waves)

auto LevelSerializer::SaveToFile(const LevelData& level, const std::filesystem::path& path) -> bool
{
    try
    {
        if (path.has_parent_path())
        {
            std::filesystem::create_directories(path.parent_path());
        }

        auto file = std::ofstream{path};
        if (!file.is_open())
        {
            std::cerr << "Failed to open level file for writing: " << path << "\n";
            return false;
        }

        const auto json = nlohmann::json(level);
        file << json.dump(4) << "\n";
        return file.good();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error saving level JSON (" << path << "): " << e.what() << "\n";
        return false;
    }
}

auto LevelSerializer::LoadFromFile(const std::filesystem::path& path) -> std::optional<LevelData>
{
    try
    {
        auto file = std::ifstream{path};
        if (!file.is_open())
        {
            std::cerr << "Failed to open level file for reading: " << path << "\n";
            return std::nullopt;
        }

        auto json = nlohmann::json{};
        file >> json;
        return json.get<LevelData>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading level JSON (" << path << "): " << e.what() << "\n";
        return std::nullopt;
    }
}
} // namespace horde
