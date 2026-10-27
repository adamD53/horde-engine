#include "level_editor.hpp"
#include "level_serializer.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

namespace horde
{
namespace
{
void CopyStringToBuffer(const std::string& source, char* buffer, size_t bufferSize)
{
    if (bufferSize == 0)
    {
        return;
    }
    const auto copyLen = std::min(source.size(), bufferSize - 1);
    std::memcpy(buffer, source.data(), copyLen);
    buffer[copyLen] = '\0';
}
} // namespace

LevelEditor::LevelEditor()
{
    const auto defaultPath = std::string{RESOURCES_PATH "levels/level_1.json"};
    CopyStringToBuffer(defaultPath, m_FilePathBuffer.data(), m_FilePathBuffer.size());
}

void LevelEditor::OnEvent(
    const sf::Event& event,
    const sf::RenderWindow& window,
    const sf::View& worldView,
    LevelData& levelData)
{
    if (m_Mode != ExecutionMode::Edit)
    {
        return;
    }

    const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>();
    if (!mousePressed || mousePressed->button != sf::Mouse::Button::Left)
    {
        return;
    }

    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    ClampSelection(levelData);
    if (!m_SelectedWave || *m_SelectedWave >= levelData.waves.size())
    {
        return;
    }

    auto& wave = levelData.waves[*m_SelectedWave];
    const auto worldPos = window.mapPixelToCoords(mousePressed->position, worldView);

    if (m_PlaceSpawnOnClick && m_SelectedSpawn && *m_SelectedSpawn < wave.spawns.size())
    {
        auto& spawn = wave.spawns[*m_SelectedSpawn];
        spawn.locationMode = SpawnLocationMode::WorldPosition;
        spawn.position = worldPos;
        return;
    }

    constexpr auto pickRadius = 20.0f;
    for (size_t i = 0; i < wave.spawns.size(); ++i)
    {
        const auto& spawn = wave.spawns[i];
        if (spawn.locationMode != SpawnLocationMode::WorldPosition)
        {
            continue;
        }

        const auto diff = spawn.position - worldPos;
        if (diff.length() <= pickRadius)
        {
            m_SelectedSpawn = i;
            break;
        }
    }
}

void LevelEditor::OnImGuiRender(LevelData& levelData)
{
    if (!m_Visible)
    {
        return;
    }

    ClampSelection(levelData);

    ImGui::SetNextWindowSize(ImVec2{680.0f, 540.0f}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Level & Wave Editor", &m_Visible))
    {
        RenderToolbar(levelData);
        ImGui::Separator();
        RenderLevelSettings(levelData);
        ImGui::Separator();

        if (ImGui::BeginTable("EditorLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Waves", ImGuiTableColumnFlags_WidthFixed, 220.0f);
            ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            RenderWaveList(levelData);

            ImGui::TableSetColumnIndex(1);
            if (m_SelectedWave && *m_SelectedWave < levelData.waves.size())
            {
                RenderWaveInspector(levelData.waves[*m_SelectedWave]);
            }
            else
            {
                ImGui::TextDisabled("Select or create a wave to inspect.");
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void LevelEditor::OnWorldRender(sf::RenderWindow& window, const LevelData& levelData) const
{
    if (m_Mode != ExecutionMode::Edit)
    {
        return;
    }

    auto playerMarker = sf::CircleShape{10.0f};
    playerMarker.setOrigin({10.0f, 10.0f});
    playerMarker.setPosition(levelData.playerSpawn);
    playerMarker.setFillColor(sf::Color{0, 200, 255, 160});
    playerMarker.setOutlineThickness(2.0f);
    playerMarker.setOutlineColor(sf::Color::White);
    window.draw(playerMarker);

    if (!m_SelectedWave || *m_SelectedWave >= levelData.waves.size())
    {
        return;
    }

    const auto& wave = levelData.waves[*m_SelectedWave];
    auto spawnMarker = sf::CircleShape{12.0f};
    spawnMarker.setOrigin({12.0f, 12.0f});
    spawnMarker.setOutlineThickness(2.0f);

    for (size_t i = 0; i < wave.spawns.size(); ++i)
    {
        const auto& spawn = wave.spawns[i];
        if (spawn.locationMode != SpawnLocationMode::WorldPosition)
        {
            continue;
        }

        const auto isSelected = m_SelectedSpawn && (*m_SelectedSpawn == i);
        spawnMarker.setPosition(spawn.position);

        if (isSelected)
        {
            spawnMarker.setFillColor(sf::Color{255, 200, 0, 200});
            spawnMarker.setOutlineColor(sf::Color::White);
        }
        else
        {
            spawnMarker.setFillColor(sf::Color{220, 60, 60, 150});
            spawnMarker.setOutlineColor(sf::Color{40, 10, 10, 220});
        }

        window.draw(spawnMarker);
    }
}

void LevelEditor::ToggleVisibility()
{
    m_Visible = !m_Visible;
}

void LevelEditor::SetVisible(bool visible)
{
    m_Visible = visible;
}

auto LevelEditor::IsVisible() const -> bool
{
    return m_Visible;
}

void LevelEditor::SetMode(ExecutionMode mode)
{
    if (m_Mode == mode)
    {
        return;
    }

    m_Mode = mode;
    if (m_OnModeChanged)
    {
        m_OnModeChanged(m_Mode);
    }
}

auto LevelEditor::GetMode() const -> ExecutionMode
{
    return m_Mode;
}

auto LevelEditor::GetSelectedWaveIndex() const -> std::optional<size_t>
{
    return m_SelectedWave;
}

void LevelEditor::SetOnTestWave(WaveTestCallback callback)
{
    m_OnTestWave = std::move(callback);
}

void LevelEditor::SetOnPlayLevel(PlayLevelCallback callback)
{
    m_OnPlayLevel = std::move(callback);
}

void LevelEditor::SetOnModeChanged(ModeChangeCallback callback)
{
    m_OnModeChanged = std::move(callback);
}

auto LevelEditor::GetCurrentFilePath() const -> std::string
{
    return std::string{m_FilePathBuffer.data()};
}

void LevelEditor::RenderToolbar(LevelData& levelData)
{
    const auto isEdit = (m_Mode == ExecutionMode::Edit);
    if (isEdit)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.45f, 0.1f, 1.0f});
        if (ImGui::Button("Mode: EDIT (Click for PLAY)"))
        {
            SetMode(ExecutionMode::Play);
        }
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.15f, 0.65f, 0.25f, 1.0f});
        if (ImGui::Button("Mode: PLAY (Click for EDIT)"))
        {
            SetMode(ExecutionMode::Edit);
        }
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    const auto canTestWave = m_SelectedWave && (*m_SelectedWave < levelData.waves.size());
    ImGui::BeginDisabled(!canTestWave);
    if (ImGui::Button("Test Selected Wave"))
    {
        if (m_OnTestWave && m_SelectedWave)
        {
            m_OnTestWave(*m_SelectedWave);
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(levelData.waves.empty());
    if (ImGui::Button("Restart Full Level"))
    {
        if (m_OnPlayLevel)
        {
            m_OnPlayLevel();
        }
    }
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::SetNextItemWidth(340.0f);
    ImGui::InputText("JSON Path", m_FilePathBuffer.data(), m_FilePathBuffer.size());

    ImGui::SameLine();
    if (ImGui::Button("Save JSON"))
    {
        const auto path = GetCurrentFilePath();
        if (LevelSerializer::SaveToFile(levelData, path))
        {
            m_StatusMessage = "Saved to " + path;
            m_StatusError = false;
        }
        else
        {
            m_StatusMessage = "Failed to save " + path;
            m_StatusError = true;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Load JSON"))
    {
        const auto path = GetCurrentFilePath();
        if (auto loaded = LevelSerializer::LoadFromFile(path))
        {
            levelData = std::move(*loaded);
            m_SelectedWave = levelData.waves.empty() ? std::nullopt : std::optional<size_t>{0};
            m_SelectedSpawn = 0;
            m_StatusMessage = "Loaded " + path;
            m_StatusError = false;
        }
        else
        {
            m_StatusMessage = "Failed to load " + path;
            m_StatusError = true;
        }
    }

    if (!m_StatusMessage.empty())
    {
        const auto color = m_StatusError ? ImVec4{1.0f, 0.35f, 0.35f, 1.0f} : ImVec4{0.4f, 0.9f, 0.4f, 1.0f};
        ImGui::TextColored(color, "%s", m_StatusMessage.c_str());
    }
}

void LevelEditor::RenderLevelSettings(LevelData& levelData)
{
    auto nameBuffer = std::array<char, 128>{};
    CopyStringToBuffer(levelData.name, nameBuffer.data(), nameBuffer.size());
    ImGui::SetNextItemWidth(220.0f);
    if (ImGui::InputText("Level Name", nameBuffer.data(), nameBuffer.size()))
    {
        levelData.name = nameBuffer.data();
    }

    ImGui::SameLine();
    auto playerPos = std::array<float, 2>{levelData.playerSpawn.x, levelData.playerSpawn.y};
    ImGui::SetNextItemWidth(180.0f);
    if (ImGui::DragFloat2("Player Spawn", playerPos.data(), 1.0f, 0.0f, 5000.0f, "%.0f"))
    {
        levelData.playerSpawn = {playerPos[0], playerPos[1]};
    }
}

void LevelEditor::RenderWaveList(LevelData& levelData)
{
    ImGui::Text("Waves (%zu)", levelData.waves.size());

    if (ImGui::Button("+ Wave"))
    {
        const auto nextIdx = levelData.waves.size() + 1;
        levelData.waves.push_back(WaveData{
            .name = "Wave " + std::to_string(nextIdx),
            .preWaveDelay = 1.0f,
            .spawns = {EnemySpawnData{}},
        });
        m_SelectedWave = levelData.waves.size() - 1;
        m_SelectedSpawn = 0;
    }

    ImGui::SameLine();
    const auto hasWave = m_SelectedWave && (*m_SelectedWave < levelData.waves.size());
    ImGui::BeginDisabled(!hasWave);
    if (ImGui::Button("Duplicate"))
    {
        auto copy = levelData.waves[*m_SelectedWave];
        copy.name += " (Copy)";
        levelData.waves.insert(
            levelData.waves.begin() + static_cast<std::ptrdiff_t>(*m_SelectedWave + 1),
            std::move(copy));
        ++(*m_SelectedWave);
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete"))
    {
        levelData.waves.erase(levelData.waves.begin() + static_cast<std::ptrdiff_t>(*m_SelectedWave));
        ClampSelection(levelData);
    }
    ImGui::EndDisabled();

    ImGui::BeginChild("WaveListChild", ImVec2{0.0f, 0.0f}, ImGuiChildFlags_Borders);
    for (size_t i = 0; i < levelData.waves.size(); ++i)
    {
        const auto isSelected = m_SelectedWave && (*m_SelectedWave == i);
        const auto label = std::to_string(i + 1) + ". " + levelData.waves[i].name + "##wave_" + std::to_string(i);
        if (ImGui::Selectable(label.c_str(), isSelected))
        {
            m_SelectedWave = i;
            m_SelectedSpawn = levelData.waves[i].spawns.empty() ? std::nullopt : std::optional<size_t>{0};
        }
    }
    ImGui::EndChild();
}

void LevelEditor::RenderWaveInspector(WaveData& wave)
{
    auto nameBuffer = std::array<char, 128>{};
    CopyStringToBuffer(wave.name, nameBuffer.data(), nameBuffer.size());
    if (ImGui::InputText("Wave Name", nameBuffer.data(), nameBuffer.size()))
    {
        wave.name = nameBuffer.data();
    }

    ImGui::DragFloat("Pre-Wave Delay (s)", &wave.preWaveDelay, 0.1f, 0.0f, 60.0f, "%.2f");

    ImGui::SeparatorText("Enemy Spawn Groups");

    if (ImGui::Button("+ Spawn Group"))
    {
        wave.spawns.push_back(EnemySpawnData{});
        m_SelectedSpawn = wave.spawns.size() - 1;
    }

    ImGui::SameLine();
    const auto hasSpawn = m_SelectedSpawn && (*m_SelectedSpawn < wave.spawns.size());
    ImGui::BeginDisabled(!hasSpawn);
    if (ImGui::Button("Duplicate Group"))
    {
        const auto copy = wave.spawns[*m_SelectedSpawn];
        wave.spawns.insert(
            wave.spawns.begin() + static_cast<std::ptrdiff_t>(*m_SelectedSpawn + 1),
            copy);
        ++(*m_SelectedSpawn);
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete Group"))
    {
        wave.spawns.erase(wave.spawns.begin() + static_cast<std::ptrdiff_t>(*m_SelectedSpawn));
        if (wave.spawns.empty())
        {
            m_SelectedSpawn = std::nullopt;
        }
        else if (*m_SelectedSpawn >= wave.spawns.size())
        {
            m_SelectedSpawn = wave.spawns.size() - 1;
        }
    }
    ImGui::EndDisabled();

    ImGui::BeginChild("SpawnGroupList", ImVec2{0.0f, 110.0f}, ImGuiChildFlags_Borders);
    for (size_t i = 0; i < wave.spawns.size(); ++i)
    {
        const auto& s = wave.spawns[i];
        const auto isSelected = m_SelectedSpawn && (*m_SelectedSpawn == i);
        const auto* modeStr = (s.locationMode == SpawnLocationMode::WorldPosition) ? "WorldPos" : "AroundPlayer";

        auto labelBuffer = std::array<char, 128>{};
        std::snprintf(
            labelBuffer.data(),
            labelBuffer.size(),
            "Group #%zu: %u x Bat [%s, delay %.1fs]##spawn_%zu",
            i + 1,
            s.count,
            modeStr,
            s.spawnDelay,
            i);

        if (ImGui::Selectable(labelBuffer.data(), isSelected))
        {
            m_SelectedSpawn = i;
        }
    }
    ImGui::EndChild();

    if (m_SelectedSpawn && *m_SelectedSpawn < wave.spawns.size())
    {
        RenderSpawnInspector(wave.spawns[*m_SelectedSpawn]);
    }
}

void LevelEditor::RenderSpawnInspector(EnemySpawnData& spawn)
{
    ImGui::SeparatorText("Selected Spawn Group");

    const char* enemyTypeNames[] = {"Bat"};
    auto currentType = static_cast<int>(spawn.type);
    if (ImGui::Combo("Enemy Type", &currentType, enemyTypeNames, 1))
    {
        spawn.type = static_cast<EnemyType>(currentType);
    }

    const char* locationModeNames[] = {"World Position (x, y)", "Around Player (Ring)"};
    auto currentMode = static_cast<int>(spawn.locationMode);
    if (ImGui::Combo("Spawn Mode", &currentMode, locationModeNames, 2))
    {
        spawn.locationMode = static_cast<SpawnLocationMode>(currentMode);
    }

    if (spawn.locationMode == SpawnLocationMode::WorldPosition)
    {
        auto pos = std::array<float, 2>{spawn.position.x, spawn.position.y};
        if (ImGui::DragFloat2("World Position", pos.data(), 2.0f, 0.0f, 5000.0f, "%.1f"))
        {
            spawn.position = {pos[0], pos[1]};
        }
        ImGui::Checkbox("Left-Click Map to Set Position (Edit Mode)", &m_PlaceSpawnOnClick);
    }

    auto count = static_cast<int>(spawn.count);
    if (ImGui::DragInt("Enemy Count", &count, 1.0f, 1, 2000))
    {
        spawn.count = static_cast<uint32_t>(std::max(1, count));
    }

    ImGui::DragFloat("Spawn Delay (s)", &spawn.spawnDelay, 0.1f, 0.0f, 120.0f, "%.2f");
    ImGui::DragFloat("Spawn Interval (s)", &spawn.spawnInterval, 0.02f, 0.0f, 10.0f, "%.2f");

    ImGui::SeparatorText("Enemy Stats Override");
    ImGui::DragFloat("Max Health", &spawn.stats.maxHealth, 1.0f, 1.0f, 5000.0f, "%.0f");
    ImGui::DragFloat("Move Speed", &spawn.stats.moveSpeed, 1.0f, 5.0f, 1000.0f, "%.0f");
    ImGui::DragFloat("Attack Damage", &spawn.stats.attackDamage, 0.5f, 0.0f, 500.0f, "%.1f");
}

void LevelEditor::ClampSelection(const LevelData& levelData)
{
    if (levelData.waves.empty())
    {
        m_SelectedWave = std::nullopt;
        m_SelectedSpawn = std::nullopt;
        return;
    }

    if (!m_SelectedWave || *m_SelectedWave >= levelData.waves.size())
    {
        m_SelectedWave = levelData.waves.size() - 1;
    }

    const auto& spawns = levelData.waves[*m_SelectedWave].spawns;
    if (spawns.empty())
    {
        m_SelectedSpawn = std::nullopt;
    }
    else if (!m_SelectedSpawn || *m_SelectedSpawn >= spawns.size())
    {
        m_SelectedSpawn = spawns.size() - 1;
    }
}
} // namespace horde
