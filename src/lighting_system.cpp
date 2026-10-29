#include "lighting_system.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>

namespace horde
{
namespace
{
struct TrigPair
{
    float cosVal;
    float sinVal;
};

constexpr auto BuildUnitCircleTable() -> std::array<TrigPair, LightingSystem::s_RayCount + 1>
{
    auto table = std::array<TrigPair, LightingSystem::s_RayCount + 1>{};
    for (size_t i = 0; i <= LightingSystem::s_RayCount; ++i)
    {
        const auto angle =
            (static_cast<float>(i) / static_cast<float>(LightingSystem::s_RayCount)) * 2.0f * std::numbers::pi_v<float>;
        table[i] = {std::cos(angle), std::sin(angle)};
    }
    return table;
}
} // namespace

LightingSystem::LightingSystem()
    : m_Vertices(sf::PrimitiveType::Triangles, s_RayCount * ((s_RingCount * 6) + 3))
{
}

auto LightingSystem::ComputeRingColor(float normalizedDist) const -> sf::Color
{
    const auto clampedDist = std::clamp(normalizedDist, 0.0f, 1.0f);
    const auto inv = 1.0f - clampedDist;
    const auto attenuation = inv * inv * (3.0f - (2.0f * inv)) * m_LightIntensity;

    const auto r =
        std::min(255.0f, static_cast<float>(m_AmbientColor.r) + (static_cast<float>(m_LightColor.r) * attenuation));
    const auto g =
        std::min(255.0f, static_cast<float>(m_AmbientColor.g) + (static_cast<float>(m_LightColor.g) * attenuation));
    const auto b =
        std::min(255.0f, static_cast<float>(m_AmbientColor.b) + (static_cast<float>(m_LightColor.b) * attenuation));

    return sf::Color{static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 255};
}

void LightingSystem::RebuildMesh(const sf::View& worldView, const sf::Vector2f& playerPos)
{
    static const auto s_UnitCircle = BuildUnitCircleTable();

    const auto centerColor = ComputeRingColor(0.0f);
    auto ringColors = std::array<sf::Color, s_RingCount>{};
    auto ringRadii = std::array<float, s_RingCount>{};

    for (size_t ring = 0; ring < s_RingCount; ++ring)
    {
        const auto t = static_cast<float>(ring + 1) / static_cast<float>(s_RingCount);
        ringRadii[ring] = m_LightRadius * t;
        ringColors[ring] = ComputeRingColor(t);
    }

    const auto viewCenter = worldView.getCenter();
    const auto viewSize = worldView.getSize();
    const auto dx = std::abs(playerPos.x - viewCenter.x) + (viewSize.x * 0.5f);
    const auto dy = std::abs(playerPos.y - viewCenter.y) + (viewSize.y * 0.5f);
    const auto outerRadius = std::max(m_LightRadius + 1.0f, std::hypot(dx, dy) * 1.05f);

    size_t idx = 0;

    for (size_t i = 0; i < s_RayCount; ++i)
    {
        const auto dir0 = sf::Vector2f{s_UnitCircle[i].cosVal, s_UnitCircle[i].sinVal};
        const auto dir1 = sf::Vector2f{s_UnitCircle[i + 1].cosVal, s_UnitCircle[i + 1].sinVal};

        const auto r0 = ringRadii[0];
        const auto p0 = playerPos + (dir0 * r0);
        const auto p1 = playerPos + (dir1 * r0);

        m_Vertices[idx++] = sf::Vertex{playerPos, centerColor};
        m_Vertices[idx++] = sf::Vertex{p0, ringColors[0]};
        m_Vertices[idx++] = sf::Vertex{p1, ringColors[0]};

        for (size_t ring = 0; ring + 1 < s_RingCount; ++ring)
        {
            const auto innerR = ringRadii[ring];
            const auto outerR = ringRadii[ring + 1];
            const auto innerCol = ringColors[ring];
            const auto outerCol = ringColors[ring + 1];

            const auto in0 = playerPos + (dir0 * innerR);
            const auto in1 = playerPos + (dir1 * innerR);
            const auto out0 = playerPos + (dir0 * outerR);
            const auto out1 = playerPos + (dir1 * outerR);

            m_Vertices[idx++] = sf::Vertex{in0, innerCol};
            m_Vertices[idx++] = sf::Vertex{out0, outerCol};
            m_Vertices[idx++] = sf::Vertex{out1, outerCol};

            m_Vertices[idx++] = sf::Vertex{in0, innerCol};
            m_Vertices[idx++] = sf::Vertex{out1, outerCol};
            m_Vertices[idx++] = sf::Vertex{in1, innerCol};
        }

        const auto lastRingR = ringRadii[s_RingCount - 1];
        const auto in0 = playerPos + (dir0 * lastRingR);
        const auto in1 = playerPos + (dir1 * lastRingR);
        const auto out0 = playerPos + (dir0 * outerRadius);
        const auto out1 = playerPos + (dir1 * outerRadius);

        m_Vertices[idx++] = sf::Vertex{in0, m_AmbientColor};
        m_Vertices[idx++] = sf::Vertex{out0, m_AmbientColor};
        m_Vertices[idx++] = sf::Vertex{out1, m_AmbientColor};

        m_Vertices[idx++] = sf::Vertex{in0, m_AmbientColor};
        m_Vertices[idx++] = sf::Vertex{out1, m_AmbientColor};
        m_Vertices[idx++] = sf::Vertex{in1, m_AmbientColor};
    }
}

void LightingSystem::Render(sf::RenderTarget& target, const sf::View& worldView, const sf::Vector2f& playerPos)
{
    if (!m_Enabled)
    {
        return;
    }

    RebuildMesh(worldView, playerPos);
    target.draw(m_Vertices, sf::BlendMultiply);
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
