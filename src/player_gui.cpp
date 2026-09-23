#include "player_gui.hpp"

#include <print>

PlayerGUI::PlayerGUI(sf::RenderWindow& window)
    : m_Window(&window)
{
    m_HealthBar.setSize({90.f, 30.f});
    m_HealthBar.setPosition({10.f, 10.f});
    m_HealthBar.setFillColor(sf::Color::Red);
    
    m_BackgroundBar.setSize({150.f, 30.f});
    m_BackgroundBar.setPosition({10.f, 10.f});
    m_BackgroundBar.setFillColor(sf::Color::Black);
}

void PlayerGUI::Update(float playerHP)
{
    std::println("Health points: {0}", playerHP);
}

void PlayerGUI::Render() const
{
    if (m_Window)
    {
        m_Window->draw(m_BackgroundBar);
        m_Window->draw(m_HealthBar);
    }
}

auto PlayerGUI::CalculateHealthBarPosition() const -> sf::Vector2f
{
    return m_HealthBar.getPosition();
}
