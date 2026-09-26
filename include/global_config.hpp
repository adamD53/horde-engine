#pragma once

#include <SFML/System/Vector2.hpp>
#include <cstdint>

namespace horde::config
{
constexpr uint32_t leftBound = 0;
constexpr uint32_t topBound = 0;
constexpr uint32_t rightBound = 1280;
constexpr uint32_t bottomBound = 1000;
constexpr uint32_t playerTextureHeight = 32;
constexpr uint32_t playerTextureWidth = 32;
constexpr float cameraZoomLevel = 0.5f;
constexpr float shootingDelay = 0.2f;
constexpr float projectileSpeed = 200.f;
constexpr sf::Vector2f playerInitialPosition{536.f, 300.f};
constexpr uint32_t initialEnemyCount = 5;
constexpr uint32_t maxEnemyCount = 100;
constexpr float minSpawnDistance = 200.f;
constexpr float maxSpawnDistance = 500.f;
constexpr uint32_t maxProjectileCount = 200;
constexpr float projectileDamage = 30.f;
}
