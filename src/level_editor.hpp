#pragma once

#include "level_data.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace horde
{
enum class ExecutionMode : uint8_t
{
    Play = 0,
    Edit = 1
};

class LevelEditor
{
public:
    using WaveTestCallback = std::function<void(size_t waveIndex)>;
    using PlayLevelCallback = std::function<void()>;
    using ModeChangeCallback = std::function<void(ExecutionMode newMode)>;

    LevelEditor();
    ~LevelEditor() = default;

    void OnEvent(
        const sf::Event& event,
        const sf::RenderWindow& window,
        const sf::View& worldView,
        LevelData& levelData);
    void OnImGuiRender(LevelData& levelData);
    void OnWorldRender(sf::RenderWindow& window, const LevelData& levelData) const;

    void ToggleVisibility();
    void SetVisible(bool visible);
    [[nodiscard]] auto IsVisible() const -> bool;

    void SetMode(ExecutionMode mode);
    [[nodiscard]] auto GetMode() const -> ExecutionMode;
    [[nodiscard]] auto GetSelectedWaveIndex() const -> std::optional<size_t>;

    void SetOnTestWave(WaveTestCallback callback);
    void SetOnPlayLevel(PlayLevelCallback callback);
    void SetOnModeChanged(ModeChangeCallback callback);

    [[nodiscard]] auto GetCurrentFilePath() const -> std::string;

private:
    void RenderToolbar(LevelData& levelData);
    void RenderLevelSettings(LevelData& levelData);
    void RenderWaveList(LevelData& levelData);
    void RenderWaveInspector(WaveData& wave);
    void RenderSpawnInspector(EnemySpawnData& spawn);
    void ClampSelection(const LevelData& levelData);

private:
    bool m_Visible{false};
    ExecutionMode m_Mode{ExecutionMode::Play};
    std::array<char, 256> m_FilePathBuffer{};
    std::string m_StatusMessage{};
    bool m_StatusError{false};

    std::optional<size_t> m_SelectedWave{0};
    std::optional<size_t> m_SelectedSpawn{0};
    bool m_PlaceSpawnOnClick{false};

    WaveTestCallback m_OnTestWave{};
    PlayLevelCallback m_OnPlayLevel{};
    ModeChangeCallback m_OnModeChanged{};
};
} // namespace horde
