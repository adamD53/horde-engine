#pragma once

#include "camera.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "player_gui.hpp"
#include "projectile.hpp"
#include "state.hpp"
#include "terrain.hpp"

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace horde
{
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

    std::vector<std::unique_ptr<Entity>> m_Entities;
    std::vector<Projectile> m_Projectiles;
    Terrain m_Terrain;
    Player m_Player;
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> m_Tilesets;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<PlayerGUI> m_PlayerGUI;
    float m_Timer{0.0f};
};
}
