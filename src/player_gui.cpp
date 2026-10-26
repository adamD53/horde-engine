#include "player_gui.hpp"

#include <algorithm>
#include <format>
#include <iostream>

namespace horde
{
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

    if (m_Font.openFromFile(RESOURCES_PATH "fonts/font.ttf"))
    {
        m_WaveText.emplace(m_Font, "", 25);
        m_WaveText->setPosition({12.f, m_BarHeight + 8.f});
    }
    else
    {
        std::cerr << "Warning: PlayerGUI could not load font from " << RESOURCES_PATH "fonts/font.ttf" << '\n';
    }
}

void PlayerGUI::Update(float currentHP, float maxHP, uint32_t waveIndex, size_t activeEnemyCount)
{
    m_CurrentHealthRatio = (maxHP > 0.0f) ? std::clamp(currentHP / maxHP, 0.0f, 1.0f) : 0.0f;
    m_HealthBar.setSize({m_WindowWidth * m_CurrentHealthRatio, m_BarHeight});

    if (m_WaveText)
    {
        m_WaveText->setString(std::format("Wave: {}\nEnemies: {}", waveIndex, activeEnemyCount));
    }
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
        if (m_WaveText)
        {
            m_Window->draw(*m_WaveText);
        }
    }
}
}
