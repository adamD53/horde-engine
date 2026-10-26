#include "game_state.hpp"
#include "defeat_state.hpp"
#include "enemy.hpp"
#include "global_config.hpp"
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
GameState::GameState(StateMachine& stateMachine, sf::RenderWindow& window)
    : m_StateMachine(stateMachine)
    , m_Window(window)
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

    m_EnemySpawner.SpawnWave(m_EnemyPool, m_Player, m_Terrain);
}

void GameState::OnEnter()
{
}

void GameState::OnExit()
{
}

void GameState::OnUpdate(float dt)
{
    const auto hasFocus = m_Window.hasFocus();
    const auto wantCapture = ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse;
    m_Player.SetInputEnabled(hasFocus && !wantCapture);
    m_Player.OnUpdate(dt);
    m_Camera->Update(m_Player.GetPos(), dt);

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
    if (hasFocus && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && m_MeleeCombatSystem.CanAttack())
    {
        const auto mousePosition =
            m_Window.mapPixelToCoords(sf::Mouse::getPosition(m_Window), m_Camera->GetViewPoint());
        const auto playerPos = m_Player.GetPos();
        const auto toMouse = sf::Vector2f{mousePosition.x - playerPos.x, mousePosition.y - playerPos.y};
        const auto aimAngle = std::atan2(toMouse.y, toMouse.x);

        m_MeleeCombatSystem.Attack(playerPos, mousePosition, m_EnemyPool);

        m_SwooshEffects.push_back(SwooshEffect{
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

    if (m_EnemyPool.GetCount() == 0)
    {
        m_EnemySpawner.SpawnWave(m_EnemyPool, m_Player, m_Terrain);
        std::cout << "Wave count: " << m_EnemyPool.GetCount() << "\n";
    }

    m_PlayerGUI->Update(
        m_Player.GetHealthPoints(),
        m_Player.GetMaxHealthPoints(),
        m_EnemySpawner.GetWaveIndex(),
        m_EnemyPool.GetCount());

    if (m_Player.IsDead())
    {
        m_StateMachine.ChangeState(std::make_unique<DefeatState>(m_StateMachine, m_Window));
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

    window.setView(m_Camera->GetUIViewPoint());
    m_PlayerGUI->Render();
}

void GameState::OnImGuiRender()
{
    // State-specific debug options can be added here
}

void GameState::OnEvent(const sf::Event& event)
{
    if (const auto* resized = event.getIf<sf::Event::Resized>())
    {
        m_Camera->OnResize(resized->size);
        m_PlayerGUI->OnResize(resized->size);
    }
    else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            m_StateMachine.ChangeState(std::make_unique<MenuState>(m_StateMachine, m_Window));
        }
    }
}
}
