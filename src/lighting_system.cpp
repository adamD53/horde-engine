#include "lighting_system.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace horde
{
LightingSystem::LightingSystem(const sf::Vector2u& windowSize)
{
    GenerateRadialGradientTexture();
    OnResize(windowSize);
}

void LightingSystem::GenerateRadialGradientTexture()
{
    auto image = sf::Image({s_GradientResolution, s_GradientResolution}, sf::Color::Transparent);
    const auto center = static_cast<float>(s_GradientResolution) * 0.5f;
    const auto maxRadius = center;

    for (uint32_t y = 0; y < s_GradientResolution; ++y)
    {
        for (uint32_t x = 0; x < s_GradientResolution; ++x)
        {
            const auto dx = (static_cast<float>(x) + 0.5f) - center;
            const auto dy = (static_cast<float>(y) + 0.5f) - center;
            const auto dist = std::sqrt((dx * dx) + (dy * dy)) / maxRadius;

            if (dist < 1.0f)
            {
                const auto inv = 1.0f - dist;
                const auto attenuation = inv * inv * (3.0f - (2.0f * inv));
                const auto alpha = static_cast<uint8_t>(std::clamp(attenuation * 255.0f, 0.0f, 255.0f));
                image.setPixel({x, y}, sf::Color{255, 255, 255, alpha});
            }
        }
    }

    if (m_RadialGradientTexture.loadFromImage(image))
    {
        m_RadialGradientTexture.setSmooth(true);
    }
}

void LightingSystem::OnResize(const sf::Vector2u& windowSize)
{
    const auto safeSize = sf::Vector2u{
        std::max(1u, windowSize.x),
        std::max(1u, windowSize.y)};
    m_LightMapReady = m_LightMap.resize(safeSize);
}

void LightingSystem::Render(sf::RenderTarget& target, const sf::View& worldView, const sf::Vector2f& playerPos)
{
    if (!m_Enabled || !m_LightMapReady)
    {
        return;
    }

    m_LightMap.clear(m_AmbientColor);
    m_LightMap.setView(worldView);

    auto lightSprite = sf::Sprite{m_RadialGradientTexture};
    const auto texSize = sf::Vector2f{m_RadialGradientTexture.getSize()};
    lightSprite.setOrigin(texSize * 0.5f);
    lightSprite.setPosition(playerPos);

    const auto diameter = m_LightRadius * 2.0f;
    lightSprite.setScale({diameter / texSize.x, diameter / texSize.y});

    const auto scaledColor = sf::Color{
        static_cast<uint8_t>(static_cast<float>(m_LightColor.r) * m_LightIntensity),
        static_cast<uint8_t>(static_cast<float>(m_LightColor.g) * m_LightIntensity),
        static_cast<uint8_t>(static_cast<float>(m_LightColor.b) * m_LightIntensity),
        255};
    lightSprite.setColor(scaledColor);

    m_LightMap.draw(lightSprite, sf::BlendAdd);
    m_LightMap.display();

    const auto previousView = target.getView();
    target.setView(target.getDefaultView());

    const auto lightMapSprite = sf::Sprite{m_LightMap.getTexture()};
    target.draw(lightMapSprite, sf::BlendMultiply);

    target.setView(previousView);
}

void LightingSystem::SetEnabled(bool enabled)
{
    m_Enabled = enabled;
}

auto LightingSystem::IsEnabled() const -> bool
{
    return m_Enabled;
}

void LightingSystem::SetAmbientColor(const sf::Color& color)
{
    m_AmbientColor = color;
}

auto LightingSystem::GetAmbientColor() const -> sf::Color
{
    return m_AmbientColor;
}

void LightingSystem::SetLightColor(const sf::Color& color)
{
    m_LightColor = color;
}

auto LightingSystem::GetLightColor() const -> sf::Color
{
    return m_LightColor;
}

void LightingSystem::SetLightRadius(float radius)
{
    m_LightRadius = std::max(1.0f, radius);
}

auto LightingSystem::GetLightRadius() const -> float
{
    return m_LightRadius;
}

void LightingSystem::SetLightIntensity(float intensity)
{
    m_LightIntensity = std::clamp(intensity, 0.0f, 1.0f);
}

auto LightingSystem::GetLightIntensity() const -> float
{
    return m_LightIntensity;
}
} // namespace horde
