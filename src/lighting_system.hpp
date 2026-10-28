#pragma once

#include "global_config.hpp"

#include <SFML/Graphics.hpp>

#include <cstdint>

namespace horde
{
class LightingSystem
{
public:
    static constexpr uint32_t s_GradientResolution = 256;

    explicit LightingSystem(const sf::Vector2u& windowSize);
    ~LightingSystem() = default;

    LightingSystem(const LightingSystem&) = delete;
    auto operator=(const LightingSystem&) -> LightingSystem& = delete;
    LightingSystem(LightingSystem&&) noexcept = default;
    auto operator=(LightingSystem&&) noexcept -> LightingSystem& = default;

    void OnResize(const sf::Vector2u& windowSize);
    void Render(sf::RenderTarget& target, const sf::View& worldView, const sf::Vector2f& playerPos);

    void SetEnabled(bool enabled);
    [[nodiscard]] auto IsEnabled() const -> bool;

    void SetAmbientColor(const sf::Color& color);
    [[nodiscard]] auto GetAmbientColor() const -> sf::Color;

    void SetLightColor(const sf::Color& color);
    [[nodiscard]] auto GetLightColor() const -> sf::Color;

    void SetLightRadius(float radius);
    [[nodiscard]] auto GetLightRadius() const -> float;

    void SetLightIntensity(float intensity);
    [[nodiscard]] auto GetLightIntensity() const -> float;

private:
    void GenerateRadialGradientTexture();

    sf::RenderTexture m_LightMap;
    sf::Texture m_RadialGradientTexture;
    sf::Color m_AmbientColor{config::ambientDarknessColor};
    sf::Color m_LightColor{config::playerLightColor};
    float m_LightRadius{config::playerLightRadius};
    float m_LightIntensity{1.0f};
    bool m_Enabled{true};
    bool m_LightMapReady{false};
};
} // namespace horde
