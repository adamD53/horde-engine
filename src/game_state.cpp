#include "game_state.hpp"
#include "enemy.hpp"
#include "global_config.hpp"
#include "menu_state.hpp"
#include "state_machine.hpp"

#include <memory>
#include <utility>

GameState::GameState(StateMachine& stateMachine, sf::RenderWindow& window)
    : m_StateMachine(stateMachine)
    , m_Window(window)
{
    m_Camera = std::make_unique<Camera>(m_Window);
    m_Window.setView(m_Camera->GetViewPoint());

    m_Tilesets.emplace(std::pair("Water", std::make_unique<sf::Texture>(RESOURCES_PATH "world/Water_tiles.png")));
    m_Tilesets.emplace(
        std::pair("Floors_Tiles", std::make_unique<sf::Texture>(RESOURCES_PATH "world/Floors_Tiles.png")));
    m_Tilesets.emplace(std::pair("Trees", std::make_unique<sf::Texture>(RESOURCES_PATH "world/Trees.png")));
    m_Tilesets.emplace(std::pair("Props", std::make_unique<sf::Texture>(RESOURCES_PATH "world/Props.png")));
    m_Tilesets.emplace(std::pair("Roofs", std::make_unique<sf::Texture>(RESOURCES_PATH "world/Roofs.png")));
    m_Tilesets.emplace(std::pair("Walls", std::make_unique<sf::Texture>(RESOURCES_PATH "world/Walls.png")));
    m_Terrain.Load(RESOURCES_PATH "world/map.tmx", m_Tilesets);
    m_Player.SetTerrain(m_Terrain);

    m_PlayerGUI = std::make_unique<PlayerGUI>(m_Window);

    for (auto i = 0; i < 4; ++i)
    {
        auto enemy = std::make_unique<Enemy>(m_Player);
        enemy->SetTerrain(m_Terrain);
        enemy->SetSwarm(&m_Entities);
        m_Entities.emplace_back(std::move(enemy));
    }
}

void GameState::OnEnter()
{
}

void GameState::OnExit()
{
}

auto GameState::CheckForBounds(const sf::Vector2f& position) const -> bool
{
    if (position.x <= config::leftBound || position.x >= config::rightBound) return true;
    if (position.y <= config::topBound || position.y >= config::bottomBound) return true;
    return false;
}

void GameState::OnUpdate(float dt)
{
    m_Player.OnUpdate(dt);
    m_Camera->SetCenterPoint(m_Player.GetPos());

    for (const auto& entity : m_Entities)
    {
        entity->OnUpdate(dt);
        m_Player.CheckForCollision(entity.get());
    }

    m_Timer += dt;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && m_Timer > config::shootingDelay)
    {
        const auto mousePosition =
            m_Window.mapPixelToCoords(sf::Mouse::getPosition(m_Window), m_Camera->GetViewPoint());
        const auto direction =
            sf::Vector2f{mousePosition.x - m_Player.GetPos().x, mousePosition.y - m_Player.GetPos().y};
        m_Projectiles.emplace_back(m_Player.GetPos(), direction);
        m_Timer = 0.0f;
    }

    for (auto& projectile : m_Projectiles)
    {
        projectile.OnUpdate(dt);
        const auto projBounds = projectile.GetBounds();
        for (const auto& entity : m_Entities)
        {
            if (!entity->IsDead() && projBounds.findIntersection(entity->GetBounds()))
            {
                constexpr auto projectileDamage = 5.f;
                entity->TakeDamage(projectileDamage);
                projectile.SetDead(true);
                break;
            }
        }
    }
    std::erase_if(m_Projectiles, [this](const auto& proj) { return proj.IsDead() || CheckForBounds(proj.GetPos()); });

    std::erase_if(m_Entities, [](const auto& entity) { return entity->IsDead(); });

    m_PlayerGUI->Update(m_Player.GetHealthPoints());
}

void GameState::OnRender(sf::RenderWindow& window)
{
    window.setView(m_Camera->GetViewPoint());

    auto depthActors = std::vector<DepthActor>{};
    depthActors.reserve(1 + m_Entities.size());

    depthActors.push_back({&m_Player, m_Player.GetPos().y});

    for (const auto& entity : m_Entities)
    {
        const auto bounds = entity->GetBounds();
        const auto feetY = bounds.position.y + bounds.size.y;
        depthActors.push_back({entity.get(), feetY});
    }

    m_Terrain.DrawWithActors(window, std::move(depthActors));

    for (const auto& projectile : m_Projectiles)
    {
        window.draw(projectile);
    }

    window.setView(window.getDefaultView());
    m_PlayerGUI->Render();
}

void GameState::OnEvent(const sf::Event& event)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            m_StateMachine.ChangeState(std::make_unique<MenuState>(m_StateMachine, m_Window));
        }
    }
}
