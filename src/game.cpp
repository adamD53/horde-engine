#include "game.hpp"
#include "SFML/System/Vector2.hpp"
#include "enemy.hpp"
#include "global_config.hpp"
#include "projectile.hpp"

#include <memory>

Game::Game(const GameSpecification& spec)
{
    m_Window.create(sf::VideoMode(sf::Vector2u(spec.width, spec.height)), spec.title);
    m_Window.setVerticalSyncEnabled(true);

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

    m_Entities.emplace_back(std::make_unique<Enemy>(m_Player));
    m_Entities.emplace_back(std::make_unique<Enemy>(m_Player));
    m_Entities.emplace_back(std::make_unique<Enemy>(m_Player));
    m_Entities.emplace_back(std::make_unique<Enemy>(m_Player));
}

auto Game::CheckForBounds(const sf::Vector2f& position) const -> bool
{
    if (position.x <= config::leftBound || position.x >= config::rightBound) return true;
    if (position.y <= config::topBound || position.y >= config::bottomBound) return true;
    return false;
}

void Game::Run()
{
    auto clock = sf::Clock{};
    auto timer = 0.0f;
    while (m_Window.isOpen())
    {
        while (const std::optional event = m_Window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) m_Window.close();
        }

        m_Window.clear();

        const auto dt = clock.restart().asSeconds();

        m_Player.OnUpdate(dt);
        m_Camera->SetCenterPoint(m_Player.GetPos());
        m_Window.setView(m_Camera->GetViewPoint());

        m_Terrain.DrawWithActor(m_Window, m_Player, m_Player.GetPos().y);

        for (const auto& entity : m_Entities)
        {
            m_Window.draw(*entity);
            entity->OnUpdate(dt);
            m_Player.CheckForCollision(entity.get());
        }

        timer += dt;
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && timer > config::shootingDelay)
        {
            const auto mousePosition = m_Window.mapPixelToCoords(sf::Mouse::getPosition(m_Window));
            const auto direction = sf::Vector2f{mousePosition.x - m_Player.GetPos().x, mousePosition.y - m_Player.GetPos().y};
            m_Projectiles.emplace_back(m_Player.GetPos(), direction);
            timer = 0.0f;
        }

        for (auto& projectile : m_Projectiles)
        {
            projectile.OnUpdate(dt);
            m_Window.draw(projectile);
        }
        std::erase_if(m_Projectiles, [this](auto& proj) { return CheckForBounds(proj.GetPos()); });

        m_Window.setView(m_Window.getDefaultView());

        m_PlayerGUI->Render();
        m_PlayerGUI->Update(m_Player.GetHealthPoints());

        m_Window.display();
    }
}
