#include "game_settings.hpp"

#include <SFML/Window/VideoMode.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>

namespace horde
{
NLOHMANN_JSON_SERIALIZE_ENUM(WindowMode, {
    {WindowMode::Windowed, "Windowed"},
    {WindowMode::Borderless, "Borderless"},
    {WindowMode::Fullscreen, "Fullscreen"},
})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Resolution, width, height)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    GameSettings,
    resolution,
    windowMode,
    vsync,
    frameRateLimit,
    masterVolume,
    musicVolume,
    sfxVolume,
    showStatsOverlay)

auto GameSettings::GetDefaultPath() -> std::filesystem::path
{
    return std::filesystem::path{RESOURCES_PATH "settings.json"};
}

auto GameSettings::GetAvailableResolutions() -> std::vector<Resolution>
{
    auto resolutions = std::vector<Resolution>{
        {1280, 720},
        {1600, 900},
        {1920, 1080},
        {2560, 1440},
        {3840, 2160},
    };

    const auto desktopMode = sf::VideoMode::getDesktopMode();
    if (desktopMode.size.x > 0 && desktopMode.size.y > 0)
    {
        resolutions.push_back({desktopMode.size.x, desktopMode.size.y});
    }

    for (const auto& mode : sf::VideoMode::getFullscreenModes())
    {
        if (mode.size.x >= 1024 && mode.size.y >= 720)
        {
            resolutions.push_back({mode.size.x, mode.size.y});
        }
    }

    std::ranges::sort(
        resolutions,
        [](const Resolution& a, const Resolution& b)
        {
            if (a.width != b.width)
            {
                return a.width < b.width;
            }
            return a.height < b.height;
        });

    const auto [firstDup, endIt] = std::ranges::unique(resolutions);
    resolutions.erase(firstDup, endIt);

    return resolutions;
}

auto SettingsSerializer::SaveToFile(const GameSettings& settings, const std::filesystem::path& path) -> bool
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
            std::cerr << "Failed to open settings file for writing: " << path << "\n";
            return false;
        }

        const auto json = nlohmann::json(settings);
        file << json.dump(4) << "\n";
        return file.good();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error saving settings JSON (" << path << "): " << e.what() << "\n";
        return false;
    }
}

auto SettingsSerializer::LoadFromFile(const std::filesystem::path& path) -> GameSettings
{
    try
    {
        if (!std::filesystem::exists(path))
        {
            const auto defaults = GameSettings{};
            (void)SaveToFile(defaults, path);
            return defaults;
        }

        auto file = std::ifstream{path};
        if (!file.is_open())
        {
            std::cerr << "Failed to open settings file for reading: " << path << "\n";
            return GameSettings{};
        }

        auto json = nlohmann::json{};
        file >> json;
        auto loaded = json.get<GameSettings>();
        loaded.masterVolume = std::clamp(loaded.masterVolume, 0.0f, 100.0f);
        loaded.musicVolume = std::clamp(loaded.musicVolume, 0.0f, 100.0f);
        loaded.sfxVolume = std::clamp(loaded.sfxVolume, 0.0f, 100.0f);
        return loaded;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading settings JSON (" << path << "): " << e.what() << "\n";
        return GameSettings{};
    }
}
} // namespace horde
