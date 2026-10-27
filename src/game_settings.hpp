#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace horde
{
enum class WindowMode : uint8_t
{
    Windowed = 0,
    Borderless = 1,
    Fullscreen = 2
};

struct Resolution
{
    uint32_t width{1920};
    uint32_t height{1080};

    [[nodiscard]] auto operator==(const Resolution& other) const -> bool = default;
};

struct GameSettings
{
    Resolution resolution{2560, 1440};
    WindowMode windowMode{WindowMode::Windowed};
    bool vsync{false};
    uint32_t frameRateLimit{60};
    float masterVolume{100.0f};
    float musicVolume{100.0f};
    float sfxVolume{100.0f};
    bool showStatsOverlay{true};

    [[nodiscard]] static auto GetDefaultPath() -> std::filesystem::path;
    [[nodiscard]] static auto GetAvailableResolutions() -> std::vector<Resolution>;
};

class SettingsSerializer
{
public:
    [[nodiscard]] static auto SaveToFile(const GameSettings& settings, const std::filesystem::path& path) -> bool;
    [[nodiscard]] static auto LoadFromFile(const std::filesystem::path& path) -> GameSettings;
};
} // namespace horde
