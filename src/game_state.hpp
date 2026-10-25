#pragma once

#include "melee_combat_system.hpp"
#include "global_config.hpp"
#include "camera.hpp"
#include "enemy.hpp"
#include "enemy_spawner.hpp"
#include "entity.hpp"
#include "object_pool.hpp"
#include "player.hpp"
#include "player_gui.hpp"
#include "spatial_grid.hpp"
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
struct SwooshEffect
{
    sf::Angle rotation;
    float currentLife{0.0f};
    float maxLife{0.15f};
};

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
    StateMachine& m_StateMachine;
    sf::RenderWindow& m_Window;

    ObjectPool<EntityVariant> m_EnemyPool;
    EnemySpawner m_EnemySpawner;
    Terrain m_Terrain;
    Player m_Player;
    MeleeCombatSystem m_MeleeCombatSystem;
    std::vector<SwooshEffect> m_SwooshEffects;
    std::vector<sf::Vector2f> m_EnemyPositions;
    SpatialGrid m_EnemyGrid;
    std::vector<DepthActor> m_DepthActors;
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> m_Tilesets;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<PlayerGUI> m_PlayerGUI;
    std::unique_ptr<sf::Sprite> m_SwooshSprite;
};
}
