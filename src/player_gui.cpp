#include "player_gui.hpp"

#include <algorithm>

PlayerGUI::PlayerGUI(sf::RenderWindow& window)
    : m_Window(&window)
    , m_WindowWidth(static_cast<float>(window.getSize().x))
{
    m_BackgroundBar.setPosition({0.f, 0.f});
    m_BackgroundBar.setSize({m_WindowWidth, m_BarHeight});
    m_BackgroundBar.setFillColor(sf::Color(30, 30, 30, 220));

    m_HealthBar.setPosition({0.f, 0.f});
    m_HealthBar.setSize({m_WindowWidth, m_BarHeight});
    m_HealthBar.setFillColor(sf::Color::Red);
}

void PlayerGUI::Update(float currentHP, float maxHP)
{
    m_CurrentHealthRatio = (maxHP > 0.0f) ? std::clamp(currentHP / maxHP, 0.0f, 1.0f) : 0.0f;
    m_HealthBar.setSize({m_WindowWidth * m_CurrentHealthRatio, m_BarHeight});
}

void PlayerGUI::OnResize(const sf::Vector2u& newSize)
{
    m_WindowWidth = static_cast<float>(newSize.x);
    m_BackgroundBar.setSize({m_WindowWidth, m_BarHeight});
    m_HealthBar.setSize({m_WindowWidth * m_CurrentHealthRatio, m_BarHeight});
}

void PlayerGUI::Render() const
{
    if (m_Window)
    {
        m_Window->draw(m_BackgroundBar);
        m_Window->draw(m_HealthBar);
    }
}
