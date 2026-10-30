#include "game_state.hpp"
#include "debug_ui.hpp"
#include "defeat_state.hpp"
#include "enemy.hpp"
#include "global_config.hpp"
#include "level_serializer.hpp"
#include "menu_state.hpp"
#include "state_machine.hpp"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <numbers>
#include <utility>

namespace horde
{
GameState::GameState(
    StateMachine& stateMachine,
    sf::RenderWindow& window,
    GameSettings& settings,
    ApplySettingsCallback onApplySettings)
    : m_StateMachine(stateMachine)
    , m_Window(window)
    , m_Settings(settings)
    , m_OnApplySettings(std::move(onApplySettings))
{
    m_Tilesets.emplace("hyptosis_tile-art-batch-1", std::make_unique<sf::Texture>(RESOURCES_PATH "world/Tiles.png"));
    m_Tilesets.emplace("Trees", std::make_unique<sf::Texture>(RESOURCES_PATH "world/Trees.png"));
    m_Tilesets.emplace("swoosh", std::make_unique<sf::Texture>(RESOURCES_PATH "weapons/swoosh.png"));
    m_SwooshSprite = std::make_unique<sf::Sprite>(*m_Tilesets.at("swoosh"));
    m_SwooshSprite->setOrigin({8.0f, 16.0f});
    m_SwooshSprite->setScale({2.5f, 2.5f});
    m_Terrain.Load(RESOURCES_PATH "world/map.tmx", m_Tilesets);
    m_Player.SetTerrain(m_Terrain);

    m_EnemyGrid.Configure(m_Terrain.GetSize(), Enemy::s_SeparationRadius);

    m_Camera = std::make_unique<Camera>(m_Window, m_Terrain.GetSize());
    m_Camera->SetCenterPoint(m_Player.GetPos());
    m_Window.setView(m_Camera->GetViewPoint());

    m_PlayerGUI = std::make_unique<PlayerGUI>(m_Window);

    m_EnemyPool.PreAllocate(config::maxEnemyCount);

    if (auto loadedLevel = LevelSerializer::LoadFromFile(m_LevelEditor.GetCurrentFilePath()))
    {
        m_LevelData = std::move(*loadedLevel);
    }
    else
    {
        m_LevelData.playerSpawn = config::playerInitialPosition;
    }

    m_LevelEditor.SetOnModeChanged([this](ExecutionMode newMode) { HandleModeChanged(newMode); });
    m_LevelEditor.SetOnTestWave([this](size_t waveIndex) { StartWaveTesting(waveIndex); });
    m_LevelEditor.SetOnPlayLevel([this]() { StartLevelFromBeginning(); });

    StartLevelFromBeginning();
}

GameState::GameState(StateMachine& stateMachine, sf::RenderWindow& window)
    : GameState(stateMachine, window, m_FallbackSettings, {})
{
    m_FallbackSettings = SettingsSerializer::LoadFromFile(GameSettings::GetDefaultPath());
}

void GameState::OnEnter()
{
}

void GameState::OnExit()
{
}

void GameState::StartLevelFromBeginning()
{
    m_IsTestingSingleWave = false;
    m_EnemyPool.Clear();
    m_SwooshEffects.clear();
    m_EnemySpawner.Reset();
    m_CurrentWaveCursor = 0;

    m_Player.SetPosition(m_LevelData.playerSpawn);
    m_Player.SetHealthPoints(m_Player.GetMaxHealthPoints());
    m_Camera->SetCenterPoint(m_Player.GetPos());

    m_LevelEditor.SetMode(ExecutionMode::Play);

    if (!m_LevelData.waves.empty())
    {
        m_EnemySpawner.StartWave(m_LevelData.waves[0], 1);
        m_CurrentWaveCursor = 1;
    }
    else
    {
        m_EnemySpawner.SpawnWave(m_EnemyPool, m_Player, m_Terrain);
    }
}

void GameState::StartWaveTesting(size_t waveIndex)
{
    if (waveIndex >= m_LevelData.waves.size())
    {
        return;
    }

    m_IsTestingSingleWave = true;
    m_EnemyPool.Clear();
    m_SwooshEffects.clear();
    m_EnemySpawner.Reset();

    m_Player.SetPosition(m_LevelData.playerSpawn);
    m_Player.SetHealthPoints(m_Player.GetMaxHealthPoints());
    m_Camera->SetCenterPoint(m_Player.GetPos());

    m_EnemySpawner.StartWave(m_LevelData.waves[waveIndex], static_cast<uint32_t>(waveIndex + 1));
    m_LevelEditor.SetMode(ExecutionMode::Play);
}

void GameState::HandleModeChanged(ExecutionMode newMode)
{
    m_LightingSystem.SetEnabled(newMode == ExecutionMode::Play);

    if (newMode == ExecutionMode::Edit)
    {
        m_IsTestingSingleWave = false;
        m_EnemyPool.Clear();
        m_SwooshEffects.clear();
        m_EnemySpawner.Reset();
        m_Player.SetInputEnabled(false);
    }
    else if (!m_IsTestingSingleWave && m_EnemyPool.GetCount() == 0 && m_EnemySpawner.IsSpawningComplete())
    {
        StartLevelFromBeginning();
    }
}

void GameState::UpdateEditModeCamera(float dt, bool inputEnabled)
{
    if (!inputEnabled)
    {
        return;
    }

    auto panDir = sf::Vector2f{0.0f, 0.0f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        panDir.x -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        panDir.x += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
    {
        panDir.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
    {
        panDir.y += 1.0f;
    }

    if (panDir.length() > 0.001f)
    {
        constexpr auto editCameraSpeed = 350.0f;
        const auto currentCenter = m_Camera->GetViewPoint().getCenter();
        m_Camera->SetCenterPoint(currentCenter + panDir.normalized() * (editCameraSpeed * dt));
    }
}

void GameState::OnUpdate(float dt)
{
    m_FrameTimeHistory[m_FrameTimeOffset] = dt * 1000.0f;
    m_FrameTimeOffset = (m_FrameTimeOffset + 1) % s_FrameTimeHistorySize;

    const auto hasFocus = m_Window.hasFocus();
    const bool imguiFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || ImGui::IsAnyItemActive();

    if (m_LevelEditor.GetMode() == ExecutionMode::Edit)
    {
        m_Player.SetInputEnabled(false);
        UpdateEditModeCamera(dt, hasFocus && !imguiFocused);
        m_PlayerGUI->Update(
            m_Player.GetHealthPoints(),
            m_Player.GetMaxHealthPoints(),
            m_EnemySpawner.GetWaveIndex(),
            m_EnemyPool.GetCount());
        return;
    }

    m_Player.SetInputEnabled(hasFocus && !imguiFocused);
    m_Player.OnUpdate(dt);
    m_Camera->Update(m_Player.GetPos(), dt);

    m_EnemySpawner.Update(dt, m_EnemyPool, m_Player, m_Terrain);

    const auto activeEnemies = m_EnemyPool.GetActive();

    m_EnemyPositions.clear();
    m_EnemyPositions.reserve(activeEnemies.size());
    for (const auto& enemyVariant : activeEnemies)
    {
        std::visit([&](const auto& enemy) { m_EnemyPositions.push_back(enemy.GetPos()); }, enemyVariant);
    }

    m_EnemyGrid.Build(m_EnemyPositions);

    for (auto& enemyVariant : activeEnemies)
    {
        std::visit(
            [&](auto& enemy)
            {
                enemy.SetSwarm(&m_EnemyGrid);
                enemy.OnUpdate(dt);
            },
            enemyVariant);
    }

    m_MeleeCombatSystem.Update(dt);
    if (hasFocus && !imguiFocused && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) &&
        m_MeleeCombatSystem.CanAttack())
    {
        const auto mousePosition =
            m_Window.mapPixelToCoords(sf::Mouse::getPosition(m_Window), m_Camera->GetViewPoint());
        const auto playerPos = m_Player.GetPos();
        const auto toMouse = sf::Vector2f{mousePosition.x - playerPos.x, mousePosition.y - playerPos.y};
        const auto aimAngle = std::atan2(toMouse.y, toMouse.x);

        m_MeleeCombatSystem.Attack(playerPos, mousePosition, m_EnemyPool);

        m_SwooshEffects.push_back(
            SwooshEffect{
                .rotation = sf::radians(aimAngle),
            });
    }

    for (auto& effect : m_SwooshEffects)
    {
        effect.currentLife += dt;
    }
    std::erase_if(m_SwooshEffects, [](const auto& effect) { return effect.currentLife >= effect.maxLife; });

    m_EnemyPool.RemoveIf([](const auto& enemyVariant)
                         { return std::visit([](const auto& enemy) { return enemy.IsDead(); }, enemyVariant); });

    if (m_EnemySpawner.IsWaveFinished(m_EnemyPool.GetCount()))
    {
        if (m_IsTestingSingleWave)
        {
            m_LevelEditor.SetMode(ExecutionMode::Edit);
        }
        else if (m_CurrentWaveCursor < m_LevelData.waves.size())
        {
            m_EnemySpawner.StartWave(
                m_LevelData.waves[m_CurrentWaveCursor], static_cast<uint32_t>(m_CurrentWaveCursor + 1));
            ++m_CurrentWaveCursor;
        }
        else
        {
            m_EnemySpawner.SpawnWave(m_EnemyPool, m_Player, m_Terrain);
            std::cout << "Wave count: " << m_EnemyPool.GetCount() << "\n";
        }
    }

    m_PlayerGUI->Update(
        m_Player.GetHealthPoints(),
        m_Player.GetMaxHealthPoints(),
        m_EnemySpawner.GetWaveIndex(),
        m_EnemyPool.GetCount());

    if (m_Player.IsDead())
    {
        if (m_IsTestingSingleWave || m_LevelEditor.IsVisible())
        {
            m_Player.SetHealthPoints(m_Player.GetMaxHealthPoints());
            m_LevelEditor.SetMode(ExecutionMode::Edit);
        }
        else
        {
            m_StateMachine.ChangeState(
                std::make_unique<DefeatState>(m_StateMachine, m_Window, m_Settings, m_OnApplySettings));
        }
    }
}

void GameState::OnRender(sf::RenderWindow& window)
{
    window.clear();
    window.setView(m_Camera->GetViewPoint());

    const auto activeEnemies = m_EnemyPool.GetActive();
    m_DepthActors.clear();
    m_DepthActors.reserve(1 + activeEnemies.size());

    const auto playerBounds = m_Player.GetBounds();
    m_DepthActors.push_back({&m_Player, playerBounds.position.y + playerBounds.size.y});

    for (const auto& enemyVariant : activeEnemies)
    {
        std::visit(
            [&](const auto& enemy)
            {
                const auto bounds = enemy.GetBounds();
                const auto feetY = bounds.position.y + bounds.size.y;
                m_DepthActors.push_back({&enemy, feetY});
            },
            enemyVariant);
    }

    m_Terrain.DrawWithActors(window, m_DepthActors);

    for (const auto& effect : m_SwooshEffects)
    {
        const auto progress = effect.currentLife / effect.maxLife;
        const auto frame = std::clamp(static_cast<int>(progress * 4.0f), 0, 3);
        m_SwooshSprite->setTextureRect(sf::IntRect({frame * 32, 0}, {32, 32}));
        m_SwooshSprite->setRotation(effect.rotation);
        m_SwooshSprite->setPosition(m_Player.GetPos());
        window.draw(*m_SwooshSprite);
    }

    m_LightingSystem.Render(window, m_Camera->GetViewPoint(), m_Player.GetPos());

    m_LevelEditor.OnWorldRender(window, m_LevelData);

    window.setView(m_Camera->GetUIViewPoint());
    m_PlayerGUI->Render();
}

void GameState::OnImGuiRender(const DebugUI& debugUI)
{
    if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_F2, false))
    {
        m_LevelEditor.ToggleVisibility();
    }

    m_LevelEditor.OnImGuiRender(m_LevelData);

    if (m_Settings.showStatsOverlay)
    {
        const auto pad = 10.0f;
        const auto* viewport = ImGui::GetMainViewport();
        const auto workPos = viewport->WorkPos;
        const auto workSize = viewport->WorkSize;
        const auto windowPos = ImVec2(workPos.x + workSize.x - pad, workPos.y + pad);
        const auto windowPosPivot = ImVec2(1.0f, 0.0f);

        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
        ImGui::SetNextWindowBgAlpha(0.65f);

        constexpr auto overlayFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

        if (ImGui::Begin("Stats Mini-HUD", &m_Settings.showStatsOverlay, overlayFlags))
        {
            const auto& io = ImGui::GetIO();
            const auto fps = io.Framerate;
            const auto frameTimeMs = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;
            ImGui::Text("%.1f FPS (%.2f ms)", fps, frameTimeMs);
            ImGui::Text(
                "Mode: %s | Wave %u",
                (m_LevelEditor.GetMode() == ExecutionMode::Edit) ? "EDIT" : "PLAY",
                m_EnemySpawner.GetWaveIndex());
            const auto playerPos = m_Player.GetPos();
            ImGui::Text("Pos: (%.0f, %.0f)", playerPos.x, playerPos.y);
        }
        ImGui::End();
    }

    if (!debugUI.IsVisible())
    {
        return;
    }

    if (ImGui::Begin("Debug UI", nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
    {
        if (ImGui::Button("Toggle Level & Wave Editor (F2)"))
        {
            m_LevelEditor.ToggleVisibility();
        }

        if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto isImmortal = m_Player.IsImmortal();
            if (ImGui::Checkbox("Immortal", &isImmortal))
            {
                m_Player.SetImmortal(isImmortal);
            }

            auto hp = m_Player.GetHealthPoints();
            if (ImGui::SliderFloat("HP", &hp, 0.0f, m_Player.GetMaxHealthPoints(), "%.0f"))
            {
                m_Player.SetHealthPoints(hp);
            }
        }

        if (ImGui::CollapsingHeader("Combat & Weapon", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto range = m_MeleeCombatSystem.GetRange();
            if (ImGui::SliderFloat("Melee Range", &range, 10.0f, 1000.0f, "%.1f"))
            {
                m_MeleeCombatSystem.SetRange(range);
            }

            auto sweepAngle = m_MeleeCombatSystem.GetSweepAngle();
            if (ImGui::SliderFloat("Sweep Angle (deg)", &sweepAngle, 10.0f, 360.0f, "%.1f"))
            {
                m_MeleeCombatSystem.SetSweepAngle(sweepAngle);
            }

            auto attackDelay = m_MeleeCombatSystem.GetAttackDelay();
            if (ImGui::SliderFloat("Attack Delay (s)", &attackDelay, 0.01f, 2.0f, "%.2f"))
            {
                m_MeleeCombatSystem.SetAttackDelay(attackDelay);
            }

            auto damage = m_MeleeCombatSystem.GetDamage();
            if (ImGui::SliderFloat("Melee Damage", &damage, 1.0f, 500.0f, "%.1f"))
            {
                m_MeleeCombatSystem.SetDamage(damage);
            }
        }

        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto enabled = m_LightingSystem.IsEnabled();
            if (ImGui::Checkbox("Enable Lighting", &enabled))
            {
                m_LightingSystem.SetEnabled(enabled);
            }

            auto radius = m_LightingSystem.GetLightRadius();
            if (ImGui::SliderFloat("Light Radius", &radius, 10.0f, 1000.0f, "%.1f"))
            {
                m_LightingSystem.SetLightRadius(radius);
            }

            auto intensity = m_LightingSystem.GetLightIntensity();
            if (ImGui::SliderFloat("Light Intensity", &intensity, 0.0f, 1.0f, "%.2f"))
            {
                m_LightingSystem.SetLightIntensity(intensity);
            }

            const auto ambient = m_LightingSystem.GetAmbientColor();
            auto ambientColor = std::array<float, 3>{
                static_cast<float>(ambient.r) / 255.0f,
                static_cast<float>(ambient.g) / 255.0f,
                static_cast<float>(ambient.b) / 255.0f};
            if (ImGui::ColorEdit3("Ambient Color", ambientColor.data()))
            {
                m_LightingSystem.SetAmbientColor(sf::Color{
                    static_cast<uint8_t>(std::clamp(ambientColor[0] * 255.0f, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(ambientColor[1] * 255.0f, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(ambientColor[2] * 255.0f, 0.0f, 255.0f)),
                    255});
            }

            const auto light = m_LightingSystem.GetLightColor();
            auto lightColor = std::array<float, 3>{
                static_cast<float>(light.r) / 255.0f,
                static_cast<float>(light.g) / 255.0f,
                static_cast<float>(light.b) / 255.0f};
            if (ImGui::ColorEdit3("Player Light Color", lightColor.data()))
            {
                m_LightingSystem.SetLightColor(sf::Color{
                    static_cast<uint8_t>(std::clamp(lightColor[0] * 255.0f, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(lightColor[1] * 255.0f, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(lightColor[2] * 255.0f, 0.0f, 255.0f)),
                    255});
            }
        }

        if (ImGui::CollapsingHeader("Performance & Statistics", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto& io = ImGui::GetIO();
            const auto fps = io.Framerate;
            const auto frameTimeMs = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;

            ImGui::Text("FPS: %.1f (%.2f ms)", fps, frameTimeMs);

            ImGui::PlotLines(
                "Frame Time",
                m_FrameTimeHistory.data(),
                static_cast<int>(m_FrameTimeHistory.size()),
                static_cast<int>(m_FrameTimeOffset),
                "ms",
                0.0f,
                33.3f,
                ImVec2(0.0f, 50.0f));

            ImGui::SeparatorText("World & Coordinates");
            ImGui::Text("Wave: %u", m_EnemySpawner.GetWaveIndex());
            const auto playerPos = m_Player.GetPos();
            ImGui::Text("Player Pos: (%.1f, %.1f)", playerPos.x, playerPos.y);
            const auto camCenter = m_Camera->GetViewPoint().getCenter();
            ImGui::Text("Camera Center: (%.1f, %.1f)", camCenter.x, camCenter.y);

            auto zoomLevel = m_Camera->GetZoomLevel();
            if (ImGui::SliderFloat(
                    "Camera Zoom", &zoomLevel, Camera::s_MinZoomLevel, m_Camera->GetMaxZoomLevel(), "%.2f"))
            {
                m_Camera->SetZoomLevel(zoomLevel);
            }
        }
    }
    ImGui::End();
}

void GameState::OnEvent(const sf::Event& event)
{
    m_LevelEditor.OnEvent(event, m_Window, m_Camera->GetViewPoint(), m_LevelData);

    if (const auto* resized = event.getIf<sf::Event::Resized>())
    {
        m_Camera->OnResize(resized->size);
        m_PlayerGUI->OnResize(resized->size);
    }
    else if (const auto* wheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        if (m_LevelEditor.GetMode() == ExecutionMode::Edit &&
            wheelScrolled->wheel == sf::Mouse::Wheel::Vertical &&
            !ImGui::GetIO().WantCaptureMouse)
        {
            constexpr auto zoomStep = 0.1f;
            const auto zoomFactor = std::pow(1.0f - zoomStep, wheelScrolled->delta);
            m_Camera->ZoomAtPixel(wheelScrolled->position, m_Window, zoomFactor);
        }
    }
    else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            m_StateMachine.ChangeState(
                std::make_unique<MenuState>(m_StateMachine, m_Window, m_Settings, m_OnApplySettings));
        }
    }
}
} // namespace horde
