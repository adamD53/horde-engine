#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <optional>

namespace horde
{
class PlayerGUI
{
public:
    PlayerGUI(sf::RenderWindow& window);
    ~PlayerGUI() = default;

    void Render() const;
    void Update(float currentHP, float maxHP, uint32_t waveIndex, size_t activeEnemyCount);
    void OnResize(const sf::Vector2u& newSize);

private:
    sf::RectangleShape m_HealthBar;
    sf::RectangleShape m_BackgroundBar;
    sf::Font m_Font;
    std::optional<sf::Text> m_WaveText;
    sf::RenderWindow* m_Window = nullptr;
    float m_BarHeight = 24.f;
    float m_WindowWidth = 0.f;
    float m_CurrentHealthRatio = 1.f;
};
}
