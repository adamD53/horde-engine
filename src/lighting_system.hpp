#pragma once

#include "global_config.hpp"

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <cstdint>

namespace horde
{
class LightingSystem
{
public:
    static constexpr size_t s_RayCount = 64;
    static constexpr size_t s_RingCount = 12;

    LightingSystem();
    ~LightingSystem() = default;

    LightingSystem(const LightingSystem&) = delete;
    auto operator=(const LightingSystem&) -> LightingSystem& = delete;
    LightingSystem(LightingSystem&&) noexcept = default;
    auto operator=(LightingSystem&&) noexcept -> LightingSystem& = default;

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
    void RebuildMesh(const sf::View& worldView, const sf::Vector2f& playerPos);
    [[nodiscard]] auto ComputeRingColor(float normalizedDist) const -> sf::Color;

    sf::VertexArray m_Vertices;
    sf::Color m_AmbientColor{config::ambientDarknessColor};
    sf::Color m_LightColor{config::playerLightColor};
    float m_LightRadius{config::playerLightRadius};
    float m_LightIntensity{1.0f};
    bool m_Enabled{true};
};
} // namespace horde
