#pragma once

#include <SFML/Graphics.hpp>

namespace horde
{
class PlayerGUI
{
public:
    PlayerGUI(sf::RenderWindow& window);
    ~PlayerGUI() = default;

    void Render() const;
    void Update(float currentHP, float maxHP);
    void OnResize(const sf::Vector2u& newSize);

private:
    sf::RectangleShape m_HealthBar;
    sf::RectangleShape m_BackgroundBar;
    sf::RenderWindow* m_Window = nullptr;
    float m_BarHeight = 24.f;
    float m_WindowWidth = 0.f;
    float m_CurrentHealthRatio = 1.f;
};
}
