#pragma once

#include "camera.hpp"
#include "enemy.hpp"
#include "enemy_spawner.hpp"
#include "entity.hpp"
#include "object_pool.hpp"
#include "player.hpp"
#include "player_gui.hpp"
#include "projectile.hpp"
#include "state.hpp"
#include "terrain.hpp"

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace horde
{
using EntityVariant = std::variant<Enemy>;

class StateMachine;

class GameState : public State
{
public:
    GameState(StateMachine& stateMachine, sf::RenderWindow& window);
    ~GameState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnRender(sf::RenderWindow& window) override;
    void OnEvent(const sf::Event& event) override;

private:
    [[nodiscard]] auto CheckForBounds(const sf::Vector2f& position) const -> bool;

private:
    StateMachine& m_StateMachine;
    sf::RenderWindow& m_Window;

    ObjectPool<EntityVariant> m_EnemyPool;
    ObjectPool<Projectile> m_ProjectilePool;
    EnemySpawner m_EnemySpawner;
    Terrain m_Terrain;
    Player m_Player;
    std::vector<sf::Vector2f> m_EnemyPositions;
    std::vector<DepthActor> m_DepthActors;
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> m_Tilesets;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<PlayerGUI> m_PlayerGUI;
    float m_Timer{0.0f};
};
}
