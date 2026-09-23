#pragma once

#include <SFML/Graphics.hpp>

class PlayerGUI
{
public:
    PlayerGUI(sf::RenderWindow& window);
    ~PlayerGUI() = default;

public:
    void Render() const;
    void Update(float playerHP);

private:
    sf::RectangleShape m_HealthBar;
    sf::RectangleShape m_BackgroundBar;
    sf::RenderWindow* m_Window = nullptr;

private:
    auto CalculateHealthBarPosition() const -> sf::Vector2f;
};
