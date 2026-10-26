#pragma once

#include "level_data.hpp"

#include <filesystem>
#include <optional>

namespace horde
{
class LevelSerializer
{
public:
    [[nodiscard]] static auto SaveToFile(const LevelData& level, const std::filesystem::path& path) -> bool;
    [[nodiscard]] static auto LoadFromFile(const std::filesystem::path& path) -> std::optional<LevelData>;
};
} // namespace horde
